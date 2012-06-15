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

#include "client.h"

#include "jobmanager.h"
#include "jsonrpc.h"
#include "transport/localsocketconnection.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QPair>
#include <QtCore/QVector>

#define DEBUGOUT(title) \
  if (this->m_debug)    \
    qDebug() << QDateTime::currentDateTime().toString() \
             << "Client" << title <<

namespace MoleQueue
{

Client::Client(QObject *parentObject) :
  AbstractRpcInterface(parentObject),
  m_jobManager(new JobManager(this)),
  m_submittedLUT(new PacketLookupTable ()),
  m_canceledLUT(new PacketLookupTable ())
{
  qRegisterMetaType<Job*>("MoleQueue::Job*");
  qRegisterMetaType<const Job*>("const MoleQueue::Job*");
  qRegisterMetaType<JobState>("MoleQueue::JobState");
  qRegisterMetaType<QueueListType>("MoleQueue::QueueListType");

  connect(m_jsonrpc, SIGNAL(queueListReceived(MoleQueue::IdType,MoleQueue::QueueListType)),
          this, SLOT(queueListReceived(MoleQueue::IdType,MoleQueue::QueueListType)));
  connect(m_jsonrpc,
          SIGNAL(successfulSubmissionReceived(MoleQueue::IdType,MoleQueue::IdType,MoleQueue::IdType,QDir)),
          this,
          SLOT(successfulSubmissionReceived(MoleQueue::IdType,MoleQueue::IdType,MoleQueue::IdType,QDir)));
  connect(m_jsonrpc,
          SIGNAL(failedSubmissionReceived(MoleQueue::IdType,MoleQueue::JobSubmissionErrorCode,QString)),
          this,
          SLOT(failedSubmissionReceived(MoleQueue::IdType,MoleQueue::JobSubmissionErrorCode,QString)));
  connect(m_jsonrpc,
          SIGNAL(jobCancellationConfirmationReceived(MoleQueue::IdType,MoleQueue::IdType)),
          this,
          SLOT(jobCancellationConfirmationReceived(MoleQueue::IdType,MoleQueue::IdType)));
  connect(m_jsonrpc, SIGNAL(jobStateChangeReceived(MoleQueue::IdType,MoleQueue::JobState,MoleQueue::JobState)),
          this, SLOT(jobStateChangeReceived(MoleQueue::IdType,MoleQueue::JobState,MoleQueue::JobState)));

  connect(m_jobManager, SIGNAL(jobAboutToBeAdded(MoleQueue::Job*)),
          this, SLOT(jobAboutToBeAdded(MoleQueue::Job*)),
          Qt::DirectConnection);
}

Client::~Client()
{
  delete m_jobManager;
  m_jobManager = NULL;

  delete m_submittedLUT;
  m_submittedLUT = NULL;

  delete m_canceledLUT;
  m_canceledLUT = NULL;
}

QueueListType Client::queueList() const
{
  return m_queueList;
}

void Client::submitJobRequest(const Job *req)
{
  const IdType id = this->nextPacketId();
  const PacketType packet = m_jsonrpc->generateJobRequest(req, id);
  m_submittedLUT->insert(id, req->clientId());
  m_connection->send(packet);
}

void Client::cancelJob(const Job *req)
{
  const IdType id = this->nextPacketId();
  const PacketType packet = m_jsonrpc->generateJobCancellation(req, id);
  m_canceledLUT->insert(id, req->clientId());
  m_connection->send(packet);
}

void Client::jobAboutToBeAdded(Job *job)
{
  job->setClientId(static_cast<IdType>(m_jobManager->count()) + 1);
}

void Client::queueListReceived(IdType, const QueueListType &list)
{
  m_queueList = list;
  emit queueListUpdated(m_queueList);
}

void Client::successfulSubmissionReceived(IdType packetId,
                                          IdType moleQueueId,
                                          IdType queueJobId,
                                          const QDir &workingDir)
{
  if (!m_submittedLUT->contains(packetId)) {
    qWarning() << "Client received a submission confirmation with an "
                  "unrecognized packet id.";
    return;
  }

  const IdType clientId = m_submittedLUT->take(packetId);
  const Job *req = m_jobManager->lookupClientId(clientId);
  if (req == NULL) {
    qWarning() << "Client received a successful job submission response for a "
                  "job that does not exist in the job list.";
    return;
  }
  Job *mutableJob = const_cast<Job*>(req);
  mutableJob->setMolequeueId(moleQueueId);
  mutableJob->setQueueJobId(queueJobId);
  mutableJob->setLocalWorkingDirectory(workingDir.absolutePath());
  m_jobManager->jobIdsChanged(req);

  emit jobSubmitted(req, true, QString());
}

void Client::failedSubmissionReceived(IdType packetId,
                                      JobSubmissionErrorCode errorCode,
                                      const QString &errorMessage)
{
  if (!m_submittedLUT->contains(packetId)) {
    qWarning() << "Client received a submission confirmation with an "
                  "unrecognized packet id.";
    return;
  }

  const IdType clientId = m_submittedLUT->take(packetId);
  const Job *req = m_jobManager->lookupClientId(clientId);
  if (req == NULL) {
    qWarning() << "Client received a failed job submission response for a "
                  "job that does not exist in the job list.";
    return;
  }

  DEBUGOUT("failedSubmissionReceived") "Job submission failed. ClientId:"
      << ((req == NULL) ? 0 : req->clientId()) << "; Error" << errorCode
      << ":" << errorMessage;

  emit jobSubmitted(req, false, errorMessage);
}

void Client::jobCancellationConfirmationReceived(IdType packetId,
                                                 IdType moleQueueId)
{
  if (!m_canceledLUT->contains(packetId)) {
    qWarning() << "Client received a submission confirmation with an "
                  "unrecognized packet id.";
    return;
  }

  const IdType clientId = m_canceledLUT->take(packetId);
  const Job *req = m_jobManager->lookupClientId(clientId);
  if (req == NULL) {
    qWarning() << "Client received a successful job cancellation response for a "
                  "job that does not exist in the job list.";
    return;
  }

  if (req->moleQueueId() != moleQueueId) {
    qWarning() << "Warning: MoleQueue id of canceled job does not match packet"
                  " id.";
  }

  emit jobCanceled(req, true, QString());
}

void Client::jobStateChangeReceived(IdType moleQueueId,
                                    JobState oldState, JobState newState)
{
  const Job *req = m_jobManager->lookupMoleQueueId(moleQueueId);
  if (req == NULL) {
    qWarning() << "Client received a job state change notification for a "
                  "job with an unrecognized MoleQueue id:" << moleQueueId;
    return;
  }

  const_cast<Job*>(req)->setJobState(newState);

  emit jobStateChanged(req, oldState, newState);
}

void Client::connectToServer(const QString &serverName)
{
  LocalSocketConnection *connection = new LocalSocketConnection(this, serverName);
  this->setConnection(connection);
  connection->open();
  connection->start();

  if (m_connection == NULL) {
    qWarning() << Q_FUNC_INFO << "Cannot connect to server at" << serverName
               << ", connection is not set.";
    return;
  }

  if (m_connection->isOpen()) {
    if (m_connection->connectionString() == serverName) {
      DEBUGOUT("connectToServer") "Socket already connected to" << serverName;
      return;
    }
    else {
      DEBUGOUT("connectToServer") "Disconnecting from server"
          << m_connection->connectionString();
      m_connection->close();
      delete m_connection;
    }
  }
  if (serverName.isEmpty()) {
    DEBUGOUT("connectToServer") "No server specified. Not attempting connection.";
    return;
  }
  else {
    m_connection = new LocalSocketConnection(this, serverName);
    connection->open();
    DEBUGOUT("connectToServer") "Client connected to server"
        << m_connection->connectionString();
  }
}

void Client::requestQueueListUpdate()
{
  PacketType packet = m_jsonrpc->generateQueueListRequest(
        this->nextPacketId());
  m_connection->send(packet);
}

Job *Client::newJobRequest()
{
  return m_jobManager->newJob();
}

}
