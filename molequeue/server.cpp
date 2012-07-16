/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "server.h"

#include "error.h"
#include "job.h"
#include "jobmanager.h"
#include "queue.h"
#include "queuemanager.h"
#include "pluginmanager.h"
#include "transport/connectionlistenerfactory.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QList>
#include <QtCore/QSettings>

#define DEBUGOUT(title) \
  if (m_debug)    \
    qDebug() << QDateTime::currentDateTime().toString() \
             << "Server" << title <<

namespace MoleQueue
{

Server::Server(QObject *parentObject, QString serverName)
  : AbstractRpcInterface(parentObject),
    m_jobManager(new JobManager (this)),
    m_queueManager(new QueueManager (this)),
    m_isTesting(false),
    m_moleQueueIdCounter(0),
    m_serverName(serverName),
    m_debug(false)
{
  qRegisterMetaType<ConnectionListener::Error>("ConnectionListener::Error");
  qRegisterMetaType<ServerConnection*>("MoleQueue::ServerConnection*");
  qRegisterMetaType<const ServerConnection*>("const MoleQueue::ServerConnection*");
  qRegisterMetaType<const Job*>("const MoleQueue::Job*");
  qRegisterMetaType<QueueListType>("MoleQueue::QueueListType");

  connect(m_jobManager, SIGNAL(jobAboutToBeAdded(MoleQueue::Job)),
          this, SLOT(jobAboutToBeAdded(MoleQueue::Job)),
          Qt::DirectConnection);

  connect(m_jobManager, SIGNAL(jobStateChanged(const MoleQueue::Job&,
                                               MoleQueue::JobState,
                                               MoleQueue::JobState)),
          this, SLOT(dispatchJobStateChange(const MoleQueue::Job&,
                                            MoleQueue::JobState,
                                            MoleQueue::JobState)));

  connect(m_jobManager, SIGNAL(jobRemoved(MoleQueue::IdType)),
          this, SLOT(jobRemoved(MoleQueue::IdType)));

  connect(m_jsonrpc, SIGNAL(queueListRequestReceived(MoleQueue::Connection*,
                                                     MoleQueue::EndpointId,
                                                     MoleQueue::IdType)),
          this, SLOT(queueListRequestReceived(MoleQueue::Connection*,
                                              MoleQueue::EndpointId,
                                              MoleQueue::IdType)));

  connect(m_jsonrpc, SIGNAL(jobSubmissionRequestReceived(MoleQueue::Connection*,
                                                         MoleQueue::EndpointId,
                                                         MoleQueue::IdType,
                                                         QVariantHash)),

          this, SLOT(jobSubmissionRequestReceived(MoleQueue::Connection*,
                                                  MoleQueue::EndpointId,
                                                  MoleQueue::IdType,
                                                  QVariantHash)));

  connect(m_jsonrpc, SIGNAL(jobCancellationRequestReceived(MoleQueue::Connection*,
                                                           MoleQueue::EndpointId,
                                                           MoleQueue::IdType,
                                                           MoleQueue::IdType)),
          this, SLOT(jobCancellationRequestReceived(MoleQueue::Connection*,
                                                    MoleQueue::EndpointId,
                                                    MoleQueue::IdType,
                                                    MoleQueue::IdType)));

  //connect(m_connection, SIGNAL(disconnected()),
  //        this, SIGNAL(disconnected()));

  // load the transport plugins so we know what to listen on
  PluginManager *pluginManager = PluginManager::instance();
  pluginManager->load();
}

Server::~Server()
{
  stop();

  delete m_jobManager;
  m_jobManager = NULL;

  delete m_queueManager;
  m_queueManager = NULL;
}

void Server::createConnectionListeners()
{
  PluginManager *pluginManager = PluginManager::instance();
  QList<ConnectionListenerFactory *> factories =
    pluginManager->connectionListenerFactories();

  foreach(ConnectionListenerFactory *factory, factories) {
    ConnectionListener *listener = factory->createConnectionListener(this,
                                                                     m_serverName);
    connect(listener, SIGNAL(connectionError(MoleQueue::ConnectionListener::Error,
                                             const QString&)),
            this, SIGNAL(connectionError(MoleQueue::ConnectionListener::Error,
                                             const QString&)));

    connect(listener, SIGNAL(newConnection(MoleQueue::Connection*)),
            this, SLOT(newConnectionAvailable(MoleQueue::Connection*)));

    m_connectionListeners.append(listener);

  }
}

void Server::readSettings(QSettings &settings)
{
  m_workingDirectoryBase = settings.value(
        "workingDirectoryBase",
        QDir::homePath() + "/.molequeue/local").toString();
  m_moleQueueIdCounter =
      settings.value("moleQueueIdCounter", 0).value<IdType>();

  m_queueManager->readSettings(settings);
  m_jobManager->readSettings(settings);
}

void Server::writeSettings(QSettings &settings) const
{
  settings.setValue("workingDirectoryBase", m_workingDirectoryBase);
  settings.setValue("moleQueueIdCounter", m_moleQueueIdCounter);

  m_queueManager->writeSettings(settings);
  m_jobManager->writeSettings(settings);
}

void Server::start()
{
  if(m_connectionListeners.empty())
    createConnectionListeners();

  foreach (ConnectionListener *listener, m_connectionListeners) {
    listener->start();
  }

  DEBUGOUT("start") "Connection listener started listening on address:"
      << m_serverName;
}

void Server::forceStart()
{
  // Force stop and restart
  stop(true);
  start();
}

void Server::stop(bool force) {

  foreach (Connection *conn, m_connections) {
    conn->close();
    delete conn;
  }

  foreach (ConnectionListener *listener, m_connectionListeners) {
    listener->stop(force);
    delete listener;
  }

  m_connections.clear();
  m_connectionListeners.clear();

}

void Server::stop()
{
  stop(false);
}

void Server::dispatchJobStateChange(const Job &job, JobState oldState,
                                    JobState newState)
{
  Connection *connection = m_connectionLUT.value(job.moleQueueId());
  EndpointId replyTo = m_endpointLUT.value(job.moleQueueId());

  sendJobStateChangeNotification(connection,
                                 replyTo,
                                 job, oldState, newState);
}

void Server::handleError(const MoleQueue::Error &err)
{
  qWarning() << "Server::handleError: Error received:\n"
                "\tType: " << err.type() << "\n"
                "\tMessage: " << err.message() << "\n"
                "\tSender: " << err.sender() << "\n"
                "\tMoleQueueId: " << err.moleQueueId();

  QString title;
  switch (err.type()) {
  default:
  case Error::MiscError:
    title = tr("MoleQueue Warning");
    break;
  case Error::NetworkError:
    title = tr("MoleQueue Network Error");
    break;
  case Error::IPCError:
    title = tr("MoleQueue Client Communication Error");
    break;
  case Error::FileSystemError:
    title = tr("MoleQueue Error");
    break;
  case Error::QueueError:
    title = tr("MoleQueue Error");
    break;
  case Error::ProgramError:
    title = tr("MoleQueue Error");
    break;
  }

  emit errorNotification(title, err.message());
}

void Server::queueListRequestReceived(MoleQueue::Connection *connection,
                                      MoleQueue::EndpointId replyTo,
                                      MoleQueue::IdType id)
{
  sendQueueList(connection, replyTo, id, m_queueManager->toQueueList());
}

void Server::jobCancellationRequestReceived(MoleQueue::Connection *connection,
                                            MoleQueue::EndpointId replyTo,
                                            MoleQueue::IdType packetId,
                                            IdType moleQueueId)
{
  m_cancellationLUT.insert(moleQueueId, packetId);

  qDebug() << "Job cancellation requested: MoleQueueId:" << moleQueueId;

   sendSuccessfulCancellationResponse(connection, replyTo, moleQueueId);
}

void Server::jobAboutToBeAdded(Job job)
{
  IdType nextMoleQueueId = ++m_moleQueueIdCounter;

  QSettings settings;
  settings.setValue("moleQueueIdCounter", m_moleQueueIdCounter);

  job.setMoleQueueId(nextMoleQueueId);
  job.setLocalWorkingDirectory(m_workingDirectoryBase + "/" +
                                QString::number(nextMoleQueueId));

  // If the outputDirectory is blank, set it now
  /// @todo Have queues check that outputdir is different from LWD before copying/cleaning.
  if (job.outputDirectory().isEmpty())
    job.setOutputDirectory(job.localWorkingDirectory());

}

void Server::newConnectionAvailable(Connection *connection)
{
  m_connections.append(connection);
  connect(connection, SIGNAL(newMessage(const MoleQueue::Message)),
          this, SLOT(readPacket(const MoleQueue::Message)));

  connect(connection, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));

  connection->start();

  DEBUGOUT("newConnectionAvailable") "New connection added:" << connection;
}

void Server::clientDisconnected()
{
  Connection *conn = qobject_cast<Connection*>(sender());
  if (conn == NULL) {
    qWarning() << Q_FUNC_INFO << "called without a ServerConnection as sender.";
    return;
  }

  DEBUGOUT("clientDisconnected") "Removing connection" << conn;
  m_connections.removeOne(conn);
  conn->deleteLater();
}

void Server::sendQueueList(Connection* connection,
                           EndpointId to,
                           MoleQueue::IdType id,
                           const QueueListType &queueList)
{
  PacketType packet = m_jsonrpc->generateQueueList(queueList, id);

  Message msg(to, packet);

  connection->send(msg);
}

void Server::sendSuccessfulSubmissionResponse(MoleQueue::Connection *connection,
                                              MoleQueue::EndpointId replyTo,
                                              const Job &job)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  const IdType moleQueueId = job.moleQueueId();
  if (!m_submissionLUT.contains(moleQueueId)) {
    qWarning() << "Refusing to confirm job submission; unrecognized MoleQueue id:"
               << moleQueueId;
    return;
  }

  const IdType packetId = m_submissionLUT.take(moleQueueId);
  PacketType packet =  m_jsonrpc->generateJobSubmissionConfirmation(
        moleQueueId, job.queueId(), job.localWorkingDirectory(),
        packetId);

  Message msg(replyTo, packet);

  connection->send(msg);
}

void Server::sendFailedSubmissionResponse(MoleQueue::Connection *connection,
                                          MoleQueue::EndpointId replyTo,
                                          const Job &job,
                                          JobSubmissionErrorCode ec,
                                          const QString &errorMessage)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  const IdType moleQueueId = job.moleQueueId();
  if (!m_submissionLUT.contains(moleQueueId)) {
    qWarning() << "Refusing to send job failure; unrecognized MoleQueue id:"
               << moleQueueId;
    return;
  }

  const IdType packetId = m_submissionLUT.take(moleQueueId);

  PacketType packet = m_jsonrpc->generateErrorResponse(static_cast<int>(ec),
                                                       errorMessage,
                                                       packetId);
  Message msg(replyTo, packet);

  connection->send(msg);
}

void Server::sendSuccessfulCancellationResponse(MoleQueue::Connection *connection,
                                                MoleQueue::EndpointId replyTo,
                                                IdType jobId)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  if (!m_cancellationLUT.contains(jobId)) {
    qWarning() << "Refusing to confirm job cancellation; unrecognized id:"
               << jobId;
    return;
  }

  const IdType packetId = m_cancellationLUT.take(jobId);
  PacketType packet =  m_jsonrpc->generateJobCancellationConfirmation(
        jobId, packetId);

  Message msg(replyTo, packet);

  connection->send(msg);
}

void Server::sendJobStateChangeNotification(MoleQueue::Connection *connection,
                                            MoleQueue::EndpointId to,
                                            const Job &job, JobState oldState,
                                            JobState newState)
{
  PacketType packet = m_jsonrpc->generateJobStateChangeNotification(
        job.moleQueueId(), oldState, newState);

  Message msg(to, packet);

  connection->send(msg);
}

void Server::jobSubmissionRequestReceived(MoleQueue::Connection *connection,
                                          MoleQueue::EndpointId replyTo,
                                          IdType packetId,
                                          const QVariantHash &options)
{
  Job job = jobManager()->newJob(options);

  m_submissionLUT.insert(job.moleQueueId(), packetId);
  m_connectionLUT.insert(job.moleQueueId(), connection);
  m_endpointLUT.insert(job.moleQueueId(), replyTo);
  m_ownedJobMoleQueueIds.append(job.moleQueueId());

  jobSubmissionRequested(connection, replyTo, job);
}

void Server::jobSubmissionRequested(MoleQueue::Connection *connection,
                                    MoleQueue::EndpointId replyTo,
                                    const Job &job)
{
  qDebug() << "Job submission requested:\n" << job.hash();

  // Lookup queue and submit job.
  Queue *queue = m_queueManager->lookupQueue(job.queue());
  if (!queue) {
    sendFailedSubmissionResponse(connection, replyTo,
                                 job, MoleQueue::InvalidQueue,
                                 tr("Unknown queue: %1").arg(job.queue()));
    return;
  }

  // Send the submission confirmation first so that the client can update the
  // MoleQueue id and properly handle packets sent during job submission.
  sendSuccessfulSubmissionResponse(connection, replyTo, job);

  /// @todo Handle submission failures better -- return JobSubErrCode?
  bool ok = queue->submitJob(job);
  qDebug() << "Submission ok?" << ok;
}

/**
 * @warning Do not dereference @a job, as it no longer points to allocated TODO clean this up
 * memory.
 */
void Server::jobRemoved(MoleQueue::IdType moleQueueId)
{
  m_connectionLUT.remove(moleQueueId);
  m_endpointLUT.remove(moleQueueId);
}

} // end namespace MoleQueue
