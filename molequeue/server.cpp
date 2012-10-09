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

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtCore/QTimerEvent>

namespace MoleQueue
{

Server::Server(QObject *parentObject, QString serverName)
  : AbstractRpcInterface(parentObject),
    m_jobManager(new JobManager (this)),
    m_queueManager(new QueueManager (this)),
    m_isTesting(false),
    m_moleQueueIdCounter(0),
    m_serverName(serverName),
    m_jobSyncTimer(startTimer(20000)) // 20 seconds
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
  if (m_jobSyncTimer != 0)
    killTimer(m_jobSyncTimer);

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

  m_queueManager->readSettings();
  m_jobManager->loadJobState(m_workingDirectoryBase + "/jobs");
}

void Server::writeSettings(QSettings &settings) const
{
  settings.setValue("workingDirectoryBase", m_workingDirectoryBase);
  settings.setValue("moleQueueIdCounter", m_moleQueueIdCounter);

  m_queueManager->writeSettings();
  m_jobManager->syncJobState();
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
  EndpointIdType endpoint = m_endpointLUT.value(job.moleQueueId());

  if (connection == NULL)
    return;

  Message msg(connection, endpoint);

  sendJobStateChangeNotification(msg, job, oldState, newState);
}

void Server::queueListRequestReceived(const Message &request)
{
  sendQueueList(request, m_queueManager->toQueueList());
}

void Server::jobCancellationRequestReceived(const Message &request,
                                            IdType moleQueueId)
{
  Job job = m_jobManager->lookupJobByMoleQueueId(moleQueueId);
  if (!job.isValid()) {
    Logger::logWarning(tr("Received cancellation request for job with invalid "
                          "MoleQueue id '%1'.").arg(moleQueueId));
    QString err(tr("Unrecognized molequeue id: %1").arg(moleQueueId));
    this->sendFailedCancellationResponse(request, moleQueueId,
                                         InvalidMoleQueueId, err);
    return;
  }

  JobState state = job.jobState();
  bool stateValid = false;
  switch (state) {
  case MoleQueue::LocalQueued:
  case MoleQueue::Submitted:
  case MoleQueue::RemoteQueued:
  case MoleQueue::RunningLocal:
  case MoleQueue::RunningRemote:
    stateValid = true;
  default:
    break;
  }

  if (!stateValid) {
    Logger::logWarning(tr("Cannot cancel job with MoleQueue id '%1': Invalid "
                          "job state ('%2').").arg(job.moleQueueId())
                       .arg(jobStateToString(state)));
    QString err(tr("Cannot kill non-running job %1 (job state: %2)")
                .arg(moleQueueId).arg(jobStateToString(state)));
    this->sendFailedCancellationResponse(request, moleQueueId,
                                         InvalidJobState, err);
    return;
  }

  Queue *queue = m_queueManager->lookupQueue(job.queue());
  if (!queue) {
    Logger::logWarning(tr("Cannot cancel job with MoleQueue id '%1': Unknown "
                          "Queue ('%2').").arg(job.moleQueueId())
                       .arg(job.queue()));
    QString err(tr("Cannot kill job %1. Unknown queue (%2)").arg(moleQueueId)
                .arg(job.queue()));
    this->sendFailedCancellationResponse(request, moleQueueId, InvalidQueue,
                                         err);
    return;
  }

  queue->killJob(job);

  sendSuccessfulCancellationResponse(request, moleQueueId);
}

void Server::lookupJobRequestReceived(const MoleQueue::Message &request,
                                      IdType moleQueueId)
{
  Job job = m_jobManager->lookupJobByMoleQueueId(moleQueueId);
  if (job.isValid())
    sendSuccessfulLookupJobResponse(request, job);
  else
    sendFailedLookupJobResponse(request, moleQueueId);
}

void Server::rpcKillRequestReceived(const Message &request)
{
  QSettings settings;
  bool enabled = settings.value("enableRpcKill", false).toBool();

  sendRpcKillResponse(request, enabled);

  if (enabled)
    qApp->quit();
}

void Server::jobAboutToBeAdded(Job job)
{
  IdType nextMoleQueueId = ++m_moleQueueIdCounter;

  QSettings settings;
  settings.setValue("moleQueueIdCounter", m_moleQueueIdCounter);

  job.setMoleQueueId(nextMoleQueueId);
  job.setLocalWorkingDirectory(m_workingDirectoryBase + "/jobs/" +
                                QString::number(nextMoleQueueId));

  // If the outputDirectory is blank, set it now
  if (job.outputDirectory().isEmpty())
    job.setOutputDirectory(job.localWorkingDirectory());

  // Create the local working directory
  if (job.localWorkingDirectory().isEmpty() ||
      !QDir().mkpath(job.localWorkingDirectory())) {
    Logger::logError(tr("Error creating working directory for job %1 "
                        "(dir='%2')").arg(job.moleQueueId())
                     .arg(job.localWorkingDirectory()),
                     job.moleQueueId());
  }
}

void Server::newConnectionAvailable(Connection *connection)
{
  m_connections.append(connection);
  connect(connection, SIGNAL(newMessage(const MoleQueue::Message)),
          this, SLOT(readMessage(const MoleQueue::Message)));

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

void Server::sendQueueList(const Message &request,
                           const QueueListType &queueList)
{
  PacketType packet = serverJsonRpc()->generateQueueList(queueList,
                                                         request.id());
  Message msg(request, packet);
  msg.send();
}

void Server::sendSuccessfulSubmissionResponse(const Message &request,
                                              const Job &job)
{
  const IdType moleQueueId = job.moleQueueId();
  PacketType packet =  serverJsonRpc()->generateJobSubmissionConfirmation(
        moleQueueId, job.localWorkingDirectory(), request.id());

  Message msg(request, packet);
  msg.send();
}

void Server::sendFailedSubmissionResponse(const Message &request,
                                          ErrorCode ec,
                                          const QString &errorMessage)
{
  PacketType packet = serverJsonRpc()->generateErrorResponse(
        static_cast<int>(ec), errorMessage, request.id());
  Message msg(request, packet);
  msg.send();
}

void Server::sendSuccessfulCancellationResponse(const Message &request,
                                                IdType moleQueueId)
{
  PacketType packet = serverJsonRpc()->generateJobCancellationConfirmation(
      moleQueueId, request.id());
  Message msg(request, packet);
  msg.send();
}

void Server::sendFailedCancellationResponse(const Message &request,
                                            IdType moleQueueId,
                                            ErrorCode error,
                                            const QString &message)
{
  PacketType packet =  serverJsonRpc()->generateJobCancellationError(
        error, message, moleQueueId, request.id());

  Message msg(request, packet);
  msg.send();
}

void Server::sendSuccessfulLookupJobResponse(const Message &request,
                                             const Job &req)
{
  PacketType packet = serverJsonRpc()->generateLookupJobResponse(
        req, req.moleQueueId(), request.id());
  Message msg(request, packet);
  msg.send();
}

void Server::sendFailedLookupJobResponse(const Message &request,
                                         IdType moleQueueId)
{
  PacketType packet = serverJsonRpc()->generateLookupJobResponse(Job(),
                                                                 moleQueueId,
                                                                 request.id());
  Message msg(request, packet);
  msg.send();
}

void Server::sendJobStateChangeNotification(const Message &connectionInfo,
                                            const Job &job, JobState oldState,
                                            JobState newState)
{
  PacketType packet = serverJsonRpc()->generateJobStateChangeNotification(
        job.moleQueueId(), oldState, newState);
  Message msg(connectionInfo, packet);
  msg.send();
}

void Server::sendRpcKillResponse(const Message &request, bool success)
{
  PacketType packet = serverJsonRpc()->generateRpcKillResponse(success,
                                                               request.id());
  Message msg(request, packet);
  msg.send();
}

void Server::jobSubmissionRequestReceived(const Message &request,
                                          const QVariantHash &options)
{
  Job job = jobManager()->newJob(options);

  m_connectionLUT.insert(job.moleQueueId(), request.connection());
  m_endpointLUT.insert(job.moleQueueId(), request.endpoint());
  m_ownedJobMoleQueueIds.append(job.moleQueueId());

  Logger::logDebugMessage(tr("Job submission requested:\n%1")
                          .arg(request.data().constData()), job.moleQueueId());

  // Lookup queue and submit job.
  Queue *queue = m_queueManager->lookupQueue(job.queue());
  if (!queue) {
    sendFailedSubmissionResponse(request, MoleQueue::InvalidQueue,
                                 tr("Unknown queue: %1").arg(job.queue()));
    Logger::logError(tr("Rejecting job: Unknown queue '%1'").arg(job.queue()),
                     job.moleQueueId());
    Job(job).setJobState(Error);
    return;
  }

  // Check program
  Program *program = queue->lookupProgram(job.program());
  if (!program) {
    sendFailedSubmissionResponse(request, MoleQueue::InvalidProgram,
                                 tr("Unknown program: %1").arg(job.program()));
    Logger::logError(tr("Rejecting job: Program '%1' Does not exist on queue "
                        "'%2'").arg(job.queue(), job.program()),
                     job.moleQueueId());
    Job(job).setJobState(Error);
    return;
  }

  // Record the new job.
  m_jobManager->syncJobState();

  // Send the submission confirmation first so that the client can update the
  // MoleQueue id and properly handle packets sent during job submission.
  sendSuccessfulSubmissionResponse(request, job);

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

void Server::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == m_jobSyncTimer) {
    e->accept();
    m_jobManager->syncJobState();
    return;
  }

  QObject::timerEvent(e);
}

void Server::setJsonRpc(JsonRpc *jsonrpc)
{
  AbstractRpcInterface::setJsonRpc(jsonrpc);

  connect(serverJsonRpc(), SIGNAL(queueListRequestReceived(MoleQueue::Message)),
          this, SLOT(queueListRequestReceived(MoleQueue::Message)));

  connect(serverJsonRpc(),
          SIGNAL(jobSubmissionRequestReceived(MoleQueue::Message,QVariantHash)),
          this,
          SLOT(jobSubmissionRequestReceived(MoleQueue::Message,QVariantHash)));

  connect(serverJsonRpc(),
          SIGNAL(jobCancellationRequestReceived(MoleQueue::Message,
                                                MoleQueue::IdType)),
          this, SLOT(jobCancellationRequestReceived(MoleQueue::Message,
                                                    MoleQueue::IdType)));

  connect(serverJsonRpc(), SIGNAL(lookupJobRequestReceived(MoleQueue::Message,
                                                           MoleQueue::IdType)),
          this, SLOT(lookupJobRequestReceived(MoleQueue::Message,
                                              MoleQueue::IdType)));
  connect(serverJsonRpc(), SIGNAL(rpcKillRequestReceived(MoleQueue::Message)),
          this, SLOT(rpcKillRequestReceived(MoleQueue::Message)));
}

} // end namespace MoleQueue
