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
  if (this->m_debug)    \
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
    m_debug(false),
    m_serverName(serverName)
{
  qRegisterMetaType<ConnectionListener::Error>("ConnectionListener::Error");
  qRegisterMetaType<ServerConnection*>("MoleQueue::ServerConnection*");
  qRegisterMetaType<const ServerConnection*>("const MoleQueue::ServerConnection*");

  connect(m_jobManager, SIGNAL(jobAboutToBeAdded(MoleQueue::Job*)),
          this, SLOT(jobAboutToBeAdded(MoleQueue::Job*)),
          Qt::DirectConnection);

  connect(m_jobManager, SIGNAL(jobStateChanged(const MoleQueue::Job*,MoleQueue::JobState,MoleQueue::JobState)),
          this, SLOT(dispatchJobStateChange(const MoleQueue::Job*,MoleQueue::JobState,MoleQueue::JobState)));

  connect(m_jobManager, SIGNAL(jobRemoved(MoleQueue::IdType, const MoleQueue::Job *)),
          this, SLOT(jobRemoved(MoleQueue::IdType, const MoleQueue::Job*)));

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

  // Route error messages back into this object's handler:
  connect(this, SIGNAL(errorOccurred(MoleQueue::Error)),
          this, SLOT(handleError(MoleQueue::Error)));

  // load the transport plugins so we know what to listener on
  PluginManager *pluginManager = PluginManager::instance();
  pluginManager->load();
}

Server::~Server()
{
  this->stop();

  qDeleteAll(m_connections);
  m_connections.clear();

  delete m_jobManager;
  m_jobManager = NULL;

  delete m_queueManager;
  m_queueManager = NULL;
}

void Server::createConnectionListener()
{
  m_serverName = (!m_isTesting) ? "MoleQueue" : "MoleQueue-testing";
  m_connectionListener = new LocalSocketConnectionListener(this, m_serverName);

  connect(m_connectionListener, SIGNAL(newConnection(MoleQueue::Connection *)),
          this, SLOT(newConnectionAvailable(MoleQueue::Connection *)));

  connect(m_connectionListener,
          SIGNAL(connectionError(MoleQueue::ConnectionListener::Error, const QString&)),
          this, SIGNAL(connectionError(MoleQueue::ConnectionListener::Error,const QString&)));

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
  this->start();
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

void Server::dispatchJobStateChange(const Job *job, JobState oldState,
                                    JobState newState)
{
  ServerConnection *conn = this->lookupConnection(job->moleQueueId());
  if (!conn)
    return;

  conn->sendJobStateChangeNotification(job, oldState, newState);
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
  ServerConnection *conn = qobject_cast<ServerConnection*>(this->sender());
  if (conn == NULL) {
    qWarning() << Q_FUNC_INFO << "called with a sender which is not a "
                  "ServerConnection.";
    return;
  }

  conn->sendQueueList(m_queueManager->toQueueList());
}

void Server::jobSubmissionRequested(const Job &job)
{
  ServerConnection *conn = qobject_cast<ServerConnection*>(this->sender());
  if (conn == NULL) {
    qWarning() << Q_FUNC_INFO << "called with a sender which is not a "
                  "ServerConnection.";
    return;
  }

  qDebug() << "Job submission requested:\n" << req->hash();

  // Lookup queue and submit job.
  Queue *queue = m_queueManager->lookupQueue(req->queue());
  if (!queue) {
    conn->sendFailedSubmissionResponse(req, MoleQueue::InvalidQueue,
                                       tr("Unknown queue: %1")
                                       .arg(req->queue()));
    return;
  }

  // Send the submission confirmation first so that the client can update the
  // MoleQueue id and properly handle packets sent during job submission.
  conn->sendSuccessfulSubmissionResponse(req);

  /// @todo Handle submission failures better -- return JobSubErrCode?
  bool ok = queue->submitJob(req);
  qDebug() << "Submission ok?" << ok;
}

void Server::jobCancellationRequested(IdType moleQueueId)
{
  ServerConnection *conn = qobject_cast<ServerConnection*>(this->sender());
  if (conn == NULL) {
    qWarning() << Q_FUNC_INFO << "called with a sender which is not a "
                  "ServerConnection.";
    return;
  }

  qDebug() << "Job cancellation requested: MoleQueueId:" << moleQueueId;

  const Job req = m_jobManager->lookupJobByMoleQueueId(moleQueueId);

  /// @todo actually handle the cancellation
  /// @todo Handle NULL req

  sendSuccessfulCancellationResponse(connection, replyTo, req);
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
  Connection *conn = qobject_cast<Connection*>(this->sender());
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
                                              const Job *req)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  const IdType moleQueueId = req->moleQueueId();
  if (!m_submissionLUT.contains(moleQueueId)) {
    qWarning() << "Refusing to confirm job submission; unrecognized MoleQueue id:"
               << moleQueueId;
    return;
  }

  const IdType packetId = m_submissionLUT.take(moleQueueId);
  PacketType packet =  m_jsonrpc->generateJobSubmissionConfirmation(
        req->moleQueueId(), req->queueJobId(), req->localWorkingDirectory(),
        packetId);

  Message msg(replyTo, packet);

  connection->send(msg);
}

void Server::sendFailedSubmissionResponse(MoleQueue::Connection *connection,
                                          MoleQueue::EndpointId replyTo,
                                          const Job *req,
                                          JobSubmissionErrorCode ec,
                                          const QString &errorMessage)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  const IdType moleQueueId = req->moleQueueId();
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
                                                const Job *req)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  const IdType moleQueueId = req->moleQueueId();
  if (!m_cancellationLUT.contains(moleQueueId)) {
    qWarning() << "Refusing to confirm job cancellation; unrecognized id:"
               << moleQueueId;
    return;
  }

  const IdType packetId = m_cancellationLUT.take(moleQueueId);
  PacketType packet =  m_jsonrpc->generateJobCancellationConfirmation(
        req->moleQueueId(), packetId);

  Message msg(replyTo, packet);

  connection->send(msg);
}

void Server::sendJobStateChangeNotification(MoleQueue::Connection *connection,
                                            MoleQueue::EndpointId to,
                                            const Job *req, JobState oldState,
                                            JobState newState)
{
  PacketType packet = m_jsonrpc->generateJobStateChangeNotification(
        req->moleQueueId(), oldState, newState);

  Message msg(to, packet);

  connection->send(msg);
}

void Server::jobSubmissionRequestReceived(MoleQueue::Connection *connection,
                                          MoleQueue::EndpointId replyTo,
                                          IdType packetId,
                                          const QVariantHash &options)
{
  Job *req = jobManager()->newJob(options);

  m_submissionLUT.insert(req->moleQueueId(), packetId);
  m_connectionLUT.insert(req->moleQueueId(), connection);
  m_endpointLUT.insert(req->moleQueueId(), replyTo);
  m_ownedJobMoleQueueIds.append(req->moleQueueId());

  jobSubmissionRequested(connection, replyTo, req);
}

void Server::jobSubmissionRequested(MoleQueue::Connection *connection,
                                    MoleQueue::EndpointId replyTo,
                                    const Job *req)
{
  qDebug() << "Job submission requested:\n" << req->hash();

  // Lookup queue and submit job.
  Queue *queue = m_queueManager->lookupQueue(req->queue());
  if (!queue) {
    sendFailedSubmissionResponse(connection, replyTo,
                                 req, MoleQueue::InvalidQueue,
                                 tr("Unknown queue: %1").arg(req->queue()));
    return;
  }

  // Send the submission confirmation first so that the client can update the
  // MoleQueue id and properly handle packets sent during job submission.
  sendSuccessfulSubmissionResponse(connection, replyTo, req);

  /// @todo Handle submission failures better -- return JobSubErrCode?
  bool ok = queue->submitJob(req);
  qDebug() << "Submission ok?" << ok;
}

/**
 * @warning Do not dereference @a job, as it no longer points to allocated TODO clean this up
 * memory.
 */
void Server::jobRemoved(MoleQueue::IdType moleQueueId, const MoleQueue::Job *job)
{
  Q_UNUSED(job);
  m_connectionLUT.remove(moleQueueId);
  m_endpointLUT.remove(moleQueueId);
}

} // end namespace MoleQueue
