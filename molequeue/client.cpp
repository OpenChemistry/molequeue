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

#include "jobrequest.h"
#include "jsonrpc.h"

#include <QtNetwork/QLocalSocket>

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
  m_jobArray(new QVector<JobRequest>()),
  m_submittedLUT(new PacketLookupTable ()),
  m_canceledLUT(new PacketLookupTable ())
{
  qRegisterMetaType<JobRequest>("JobRequest");

  QLocalSocket *socket = new QLocalSocket ();
  socket->connectToServer("MoleQueue");
  this->setSocket(socket);

  connect(m_jsonrpc, SIGNAL(queueListReceived(IdType,QueueListType)),
          this, SLOT(queueListReceived(IdType,QueueListType)));
  connect(m_jsonrpc,
          SIGNAL(successfulSubmissionReceived(IdType,IdType,IdType,QDir)),
          this,
          SLOT(successfulSubmissionReceived(IdType,IdType,IdType,QDir)));
  connect(m_jsonrpc,
          SIGNAL(failedSubmissionReceived(IdType,JobSubmissionErrorCode,QString)),
          this,
          SLOT(failedSubmissionReceived(IdType,JobSubmissionErrorCode,QString)));
  connect(m_jsonrpc,
          SIGNAL(jobCancellationConfirmationReceived(IdType,IdType)),
          this,
          SLOT(jobCancellationConfirmationReceived(IdType,IdType)));
  connect(m_jsonrpc, SIGNAL(jobStateChangeReceived(IdType,JobState,JobState)),
          this, SLOT(jobStateChangeReceived(IdType,JobState,JobState)));
}

Client::~Client()
{
  delete m_jobArray;
  m_jobArray = NULL;

  delete m_submittedLUT;
  m_submittedLUT = NULL;

  delete m_canceledLUT;
  m_canceledLUT = NULL;
}

JobRequest &Client::newJobRequest()
{
  m_jobArray->push_back(JobRequest (this));
  JobRequest &newJob = m_jobArray->back();
  newJob.setClientId(m_jobArray->size());
  return newJob;
}

QueueListType Client::queueList() const
{
  return m_queueList;
}

void Client::submitJobRequest(const JobRequest &req)
{
  const IdType id = this->nextPacketId();
  const PacketType packet = m_jsonrpc->generateJobRequest(req, id);
  m_submittedLUT->insert(id, req.clientId());
  this->sendPacket(packet);
}

void Client::cancelJobRequest(const JobRequest &req)
{
  const IdType id = this->nextPacketId();
  const PacketType packet = m_jsonrpc->generateJobCancellation(req, id);
  m_canceledLUT->insert(id, req.clientId());
  this->sendPacket(packet);
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
  JobRequest *req = this->jobRequestByClientId(clientId);
  if (req == NULL) {
    qWarning() << "Client received a successful job submission response for a "
                  "job that does not exist in the job list.";
    return;
  }
  req->setMolequeueId(moleQueueId);
  req->setQueueJobId(queueJobId);
  req->setLocalWorkingDirectory(workingDir.absolutePath());

  emit jobSubmitted(*req, true, QString());
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
  JobRequest *req = this->jobRequestByClientId(clientId);
  if (req == NULL) {
    qWarning() << "Client received a failed job submission response for a "
                  "job that does not exist in the job list.";
    return;
  }

  DEBUGOUT("failedSubmissionReceived") "Job submission failed. ClientId:"
      << ((req == NULL) ? 0 : req->clientId()) << "; Error" << errorCode
      << ":" << errorMessage;

  emit jobSubmitted(*req, false, errorMessage);
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
  JobRequest *req = this->jobRequestByClientId(clientId);
  if (req == NULL) {
    qWarning() << "Client received a successful job cancellation response for a "
                  "job that does not exist in the job list.";
    return;
  }

  if (req->moleQueueId() != moleQueueId) {
    qWarning() << "Warning: MoleQueue id of canceled job does not match packet"
                  " id.";
  }

  emit jobCanceled(*req, true, QString());
}

void Client::jobStateChangeReceived(IdType moleQueueId,
                                    JobState oldState, JobState newState)
{
  JobRequest *req = this->jobRequestByMoleQueueId(moleQueueId);
  if (req == NULL) {
    qWarning() << "Client received a job state change notification for a "
                  "job with an unrecognized MoleQueue id:" << moleQueueId;
    return;
  }

  req->setJobState(newState);

  emit jobStateChanged(*req, oldState, newState);
}

void Client::requestQueueListUpdate()
{
  PacketType packet = m_jsonrpc->generateQueueListRequest(
        this->nextPacketId());
  this->sendPacket(packet);
}

JobRequest * Client::jobRequestByClientId(IdType clientId)
{
  if (clientId < 1 || clientId > static_cast<IdType>(m_jobArray->size())) {
    qWarning() << "Request for JobRequest with invalid clientId:" << clientId;
    return NULL;
  }
  return &((*m_jobArray)[clientId - 1]);
}

const JobRequest * Client::jobRequestByClientId(IdType clientId) const
{
  if (clientId < 1 || clientId > static_cast<IdType>(m_jobArray->size())) {
    qWarning() << "Request for JobRequest with invalid clientId:" << clientId;
    return NULL;
  }
  return &((*m_jobArray)[clientId - 1]);
}

JobRequest *Client::jobRequestByMoleQueueId(IdType moleQueueId)
{
  for (QVector<JobRequest>::const_iterator it = m_jobArray->constBegin(),
       it_end = m_jobArray->constEnd(); it != it_end; ++it) {
    if (it->moleQueueId() != moleQueueId)
      continue;
    // Look up with getRequestByClientId to get mutable ref
    return this->jobRequestByClientId(it->clientId());
  }

  qWarning() << "Request for JobRequest with unrecognized moleQueueId:"
             << moleQueueId;
  return NULL;
}

const JobRequest *Client::jobRequestByMoleQueueId(IdType moleQueueId) const
{
  for (QVector<JobRequest>::const_iterator it = m_jobArray->constBegin(),
       it_end = m_jobArray->constEnd(); it != it_end; ++it) {
    if (it->moleQueueId() != moleQueueId)
      continue;
    return &(*it);
  }

  qWarning() << "Request for JobRequest with unrecognized moleQueueId:"
             << moleQueueId;
  return NULL;
}

}
