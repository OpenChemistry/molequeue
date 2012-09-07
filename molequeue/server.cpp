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

#include "job.h"
#include "jobmanager.h"
#include "logger.h"
#include "queue.h"
#include "queuemanager.h"
#include "serverjsonrpc.h"
#include "pluginmanager.h"
#include "transport/connectionlistenerfactory.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QList>
#include <QtCore/QSettings>

namespace MoleQueue
{

Server::Server(QObject *parentObject, QString serverName)
  : AbstractRpcInterface(parentObject),
    m_jobManager(new JobManager (this)),
    m_queueManager(new QueueManager (this)),
    m_isTesting(false),
    m_moleQueueIdCounter(0),
    m_serverName(serverName)
{
  qRegisterMetaType<ConnectionListener::Error>("ConnectionListener::Error");
  qRegisterMetaType<ServerConnection*>("MoleQueue::ServerConnection*");
  qRegisterMetaType<const ServerConnection*>("const MoleQueue::ServerConnection*");
  qRegisterMetaType<const Job*>("const MoleQueue::Job*");
  qRegisterMetaType<QueueListType>("MoleQueue::QueueListType");

  this->setJsonRpc(new ServerJsonRpc(this));

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

  Logger::logDebugMessage(tr("Server started listening on address '%1'")
                          .arg(m_serverName));
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

void Server::stop() {
  stop(false);
}

void Server::dispatchJobStateChange(const Job &job, JobState oldState,
                                    JobState newState)
{
  Connection *connection = m_connectionLUT.value(job.moleQueueId());
  EndpointId replyTo = m_endpointLUT.value(job.moleQueueId());

  if (connection == NULL)
    return;

  sendJobStateChangeNotification(connection,
                                 replyTo,
                                 job, oldState, newState);
}

void Server::queueListRequestReceived(MoleQueue::Connection *connection,
                                      MoleQueue::EndpointId replyTo,
                                      MoleQueue::IdType packetId)
{
  sendQueueList(connection, replyTo, packetId, m_queueManager->toQueueList());
}

void Server::jobCancellationRequestReceived(MoleQueue::Connection *connection,
                                            MoleQueue::EndpointId replyTo,
                                            MoleQueue::IdType packetId,
                                            IdType moleQueueId)
{
  m_cancellationLUT.insert(moleQueueId, packetId);

  Job job = m_jobManager->lookupJobByMoleQueueId(moleQueueId);
  if (!job.isValid()) {
    Logger::logWarning(tr("Received cancellation request for job with invalid "
                          "MoleQueue id '%1'.").arg(moleQueueId));
  }
  else {
    Queue *queue = m_queueManager->lookupQueue(job.queue());
    if (!queue) {
      Logger::logWarning(tr("Cannot cancel job with MoleQueue id '%1': Unknown "
                            "Queue ('%2').").arg(job.moleQueueId())
                         .arg(job.queue()));
    }
    else {
      queue->killJob(job);
    }
  }

  sendSuccessfulCancellationResponse(connection, replyTo, moleQueueId);
}

void Server::lookupJobRequestReceived(Connection *connection,
                                      EndpointId replyTo, IdType packetId,
                                      IdType moleQueueId)
{
  Job job = m_jobManager->lookupJobByMoleQueueId(moleQueueId);
  if (job.isValid())
    sendSuccessfulLookupJobResponse(connection, replyTo, packetId, job);
  else
    sendFailedLookupJobResponse(connection, replyTo, packetId, moleQueueId);
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

  Logger::logDebugMessage(tr("Client connected: %1")
                          .arg(connection->connectionString()));
}

void Server::clientDisconnected()
{
  Connection *conn = qobject_cast<Connection*>(sender());
  if (conn == NULL)
    return;

  Logger::logDebugMessage(tr("Client disconnected: %1")
                          .arg(conn->connectionString()));

  m_connections.removeOne(conn);

  // Remove connection from look up table and any endpoints key on molequeueids
  // associated with that connection.
  QList<IdType> moleQueueIds = m_connectionLUT.keys(conn);

  foreach(IdType moleQueueId, moleQueueIds) {
    m_connectionLUT.remove(moleQueueId);
    m_endpointLUT.remove(moleQueueId);
  }

  conn->deleteLater();
}

void Server::sendQueueList(Connection* connection,
                           EndpointId to,
                           MoleQueue::IdType packetId,
                           const QueueListType &queueList)
{
  PacketType packet = serverJsonRpc()->generateQueueList(queueList, packetId);

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
    Logger::logWarning(tr("Cannot confirm job submission for unrecognized "
                          "MoleQueue id '%1'").arg(moleQueueId), moleQueueId);
    return;
  }

  const IdType packetId = m_submissionLUT.take(moleQueueId);
  PacketType packet =  serverJsonRpc()->generateJobSubmissionConfirmation(
        moleQueueId, job.localWorkingDirectory(), packetId);

  Message msg(replyTo, packet);

  connection->send(msg);
}

void Server::sendFailedSubmissionResponse(MoleQueue::Connection *connection,
                                          MoleQueue::EndpointId replyTo,
                                          const Job &job,
                                          ErrorCode ec,
                                          const QString &errorMessage)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  const IdType moleQueueId = job.moleQueueId();
  if (!m_submissionLUT.contains(moleQueueId)) {
    Logger::logWarning(tr("Cannot send job failure message for unrecognized "
                          "MoleQueue id '%1'").arg(moleQueueId), moleQueueId);
    return;
  }

  const IdType packetId = m_submissionLUT.take(moleQueueId);

  PacketType packet = serverJsonRpc()->generateErrorResponse(
        static_cast<int>(ec), errorMessage, packetId);
  Message msg(replyTo, packet);

  connection->send(msg);
}

void Server::sendSuccessfulCancellationResponse(MoleQueue::Connection *connection,
                                                MoleQueue::EndpointId replyTo,
                                                IdType moleQueueId)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  if (!m_cancellationLUT.contains(moleQueueId)) {
    Logger::logWarning(tr("Cannot confirm job cancellation for unrecognized "
                          "MoleQueue id '%1'").arg(moleQueueId), moleQueueId);
    return;
  }

  const IdType packetId = m_cancellationLUT.take(moleQueueId);
  PacketType packet = serverJsonRpc()->generateJobCancellationConfirmation(
      moleQueueId, packetId);

  Message msg(replyTo, packet);

  connection->send(msg);
}

void Server::sendSuccessfulLookupJobResponse(Connection *connection,
                                             EndpointId replyTo,
                                             IdType packetId, const Job &req)
{
  PacketType packet = serverJsonRpc()->generateLookupJobResponse(
        req, req.moleQueueId(), packetId);
  Message msg(replyTo, packet);
  connection->send(msg);
}

void Server::sendFailedLookupJobResponse(Connection *connection,
                                         EndpointId replyTo, IdType packetId,
                                         IdType moleQueueId)
{
  PacketType packet = serverJsonRpc()->generateLookupJobResponse(Job(),
                                                                 moleQueueId,
                                                                 packetId);
  Message msg(replyTo, packet);
  connection->send(msg);
}

void Server::sendJobStateChangeNotification(MoleQueue::Connection *connection,
                                            MoleQueue::EndpointId to,
                                            const Job &job, JobState oldState,
                                            JobState newState)
{
  PacketType packet = serverJsonRpc()->generateJobStateChangeNotification(
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
  QString stateString;
  QVariantHash jobHash = job.hash();
  foreach (const QString &key, jobHash.keys()) {
    stateString += QString("%1: '%2' ").arg(key)
        .arg(jobHash.value(key).toString());
  }
  Logger::logDebugMessage(tr("Job submission requested:\n%1").arg(stateString));

  // Lookup queue and submit job.
  Queue *queue = m_queueManager->lookupQueue(job.queue());
  if (!queue) {
    sendFailedSubmissionResponse(connection, replyTo,
                                 job, MoleQueue::InvalidQueue,
                                 tr("Unknown queue: %1").arg(job.queue()));
    Logger::logError(tr("Rejecting job: Unknown queue '%1'").arg(job.queue()),
                     job.moleQueueId());
    Job(job).setJobState(Error);
    return;
  }

  // Check program
  Program *program = queue->lookupProgram(job.program());
  if (!program) {
    sendFailedSubmissionResponse(connection, replyTo,
                                 job, MoleQueue::InvalidProgram,
                                 tr("Unknown program: %1").arg(job.program()));
    Logger::logError(tr("Rejecting job: Program '%1' Does not exist on queue "
                        "'%2'").arg(job.queue(), job.program()),
                     job.moleQueueId());
    Job(job).setJobState(Error);
    return;
  }

  // Send the submission confirmation first so that the client can update the
  // MoleQueue id and properly handle packets sent during job submission.
  sendSuccessfulSubmissionResponse(connection, replyTo, job);

  if (!queue->submitJob(job)) {
    Logger::logError(tr("Error starting job! (Refused by queue)"),
                     job.moleQueueId());
    Job(job).setJobState(Error);
  }
}

void Server::jobRemoved(MoleQueue::IdType moleQueueId)
{
  m_connectionLUT.remove(moleQueueId);
  m_endpointLUT.remove(moleQueueId);
}

void Server::setJsonRpc(JsonRpc *jsonrpc)
{
  AbstractRpcInterface::setJsonRpc(jsonrpc);

  connect(serverJsonRpc(),
          SIGNAL(queueListRequestReceived(MoleQueue::Connection*,
                                          MoleQueue::EndpointId,
                                          MoleQueue::IdType)),
          this,
          SLOT(queueListRequestReceived(MoleQueue::Connection*,
                                        MoleQueue::EndpointId,
                                        MoleQueue::IdType)));

  connect(serverJsonRpc(),
          SIGNAL(jobSubmissionRequestReceived(MoleQueue::Connection*,
                                              MoleQueue::EndpointId,
                                              MoleQueue::IdType,
                                              QVariantHash)),
          this,
          SLOT(jobSubmissionRequestReceived(MoleQueue::Connection*,
                                            MoleQueue::EndpointId,
                                            MoleQueue::IdType,
                                            QVariantHash)));

  connect(serverJsonRpc(),
          SIGNAL(jobCancellationRequestReceived(MoleQueue::Connection*,
                                                MoleQueue::EndpointId,
                                                MoleQueue::IdType,
                                                MoleQueue::IdType)),
          this,
          SLOT(jobCancellationRequestReceived(MoleQueue::Connection*,
                                              MoleQueue::EndpointId,
                                              MoleQueue::IdType,
                                              MoleQueue::IdType)));

  connect(serverJsonRpc(),
          SIGNAL(lookupJobRequestReceived(MoleQueue::Connection*,
                                          MoleQueue::EndpointId,
                                          MoleQueue::IdType,
                                          MoleQueue::IdType)),
          this,
          SLOT(lookupJobRequestReceived(MoleQueue::Connection*,
                                        MoleQueue::EndpointId,
                                        MoleQueue::IdType,
                                        MoleQueue::IdType)));
}

} // end namespace MoleQueue
