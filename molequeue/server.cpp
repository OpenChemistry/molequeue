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
#include "queue.h"
#include "queuemanager.h"
#include "serverconnection.h"
#include "transport/localsocketconnectionlistener.h"

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

Server::Server(QObject *parentObject)
  : QObject(parentObject),
    m_connectionListener(NULL),
    m_jobManager(new JobManager (this)),
    m_queueManager(new QueueManager (this)),
    m_isTesting(false),
    m_moleQueueIdCounter(0),
    m_debug(false)
{
  qRegisterMetaType<ConnectionListener::Error>("ConnectionListener::Error");
  qRegisterMetaType<ServerConnection*>("MoleQueue::ServerConnection*");
  qRegisterMetaType<const ServerConnection*>("const MoleQueue::ServerConnection*");

  connect(m_jobManager, SIGNAL(jobAboutToBeAdded(MoleQueue::Job*)),
          this, SLOT(jobAboutToBeAdded(MoleQueue::Job*)),
          Qt::DirectConnection);

  connect(m_jobManager, SIGNAL(jobStateChanged(const MoleQueue::Job*,MoleQueue::JobState,MoleQueue::JobState)),
          this, SLOT(dispatchJobStateChange(const MoleQueue::Job*,MoleQueue::JobState,MoleQueue::JobState)));
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
  if(m_connectionListener == NULL)
    createConnectionListener();

  m_connectionListener->start();

  DEBUGOUT("start") "Connection listener started listening on address:"
      << m_serverName;
}

void Server::forceStart()
{
  // Try a new connection listener
  if(m_connectionListener)
    m_connectionListener->stop(true);

  delete m_connectionListener;
  createConnectionListener();

  this->start();
}

void Server::stop()
{
  if (m_connectionListener) {
    m_connectionListener->stop();
    delete m_connectionListener;
    m_connectionListener = NULL;
  }
}

void Server::dispatchJobStateChange(const Job *job,
                                    JobState oldState, JobState newState)
{
  ServerConnection *conn = this->lookupConnection(job->moleQueueId());
  if (!conn)
    return;

  conn->sendJobStateChangeNotification(job, oldState, newState);
}

void Server::queueListRequested()
{
  ServerConnection *conn = qobject_cast<ServerConnection*>(this->sender());
  if (conn == NULL) {
    qWarning() << Q_FUNC_INFO << "called with a sender which is not a "
                  "ServerConnection.";
    return;
  }

  conn->sendQueueList(m_queueManager->toQueueList());
}

void Server::jobSubmissionRequested(const Job *req)
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

  const Job *req = m_jobManager->lookupMoleQueueId(moleQueueId);

  /// @todo actually handle the cancellation
  /// @todo Handle NULL req

  conn->sendSuccessfulCancellationResponse(req);
}

void Server::jobAboutToBeAdded(Job *job)
{
  IdType nextMoleQueueId = ++m_moleQueueIdCounter;

  QSettings settings;
  settings.setValue("moleQueueIdCounter", m_moleQueueIdCounter);

  job->setMolequeueId(nextMoleQueueId);
  job->setLocalWorkingDirectory(m_workingDirectoryBase + "/" +
                                QString::number(nextMoleQueueId));
}

void Server::newConnectionAvailable(Connection *connection)
{
  ServerConnection *conn = new ServerConnection (this, connection);

  connect(conn, SIGNAL(queueListRequested()), this, SLOT(queueListRequested()));
  connect(conn, SIGNAL(jobSubmissionRequested(const MoleQueue::Job*)),
          this, SLOT(jobSubmissionRequested(const MoleQueue::Job*)));
  connect(conn, SIGNAL(jobCancellationRequested(MoleQueue::IdType)),
          this, SLOT(jobCancellationRequested(MoleQueue::IdType)));

  m_connections.append(conn);
  connect(conn, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
 emit newConnection(conn);
  DEBUGOUT("newConnectionAvailable") "New connection added:" << conn;
  conn->startProcessing();
}

void Server::clientDisconnected()
{
  ServerConnection *conn = qobject_cast<ServerConnection*>(this->sender());
  if (conn == NULL) {
    qWarning() << Q_FUNC_INFO << "called without a ServerConnection as sender.";
    return;
  }

  DEBUGOUT("clientDisconnected") "Removing connection" << conn;
  m_connections.removeOne(conn);
  conn->deleteLater();
}

ServerConnection *Server::lookupConnection(IdType moleQueueId)
{
  foreach (ServerConnection *conn, m_connections) {
    if (!conn->hasJob(moleQueueId))
      continue;
    return conn;
  }

  return NULL;
}

} // end namespace MoleQueue
