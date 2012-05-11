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
  m_submittedLUT(new PacketLookupTable ())
{
  connect(m_jsonrpc, SIGNAL(queueListReceived(mqIdType,mqQueueListType)),
          this, SLOT(queueListReceived(mqIdType,mqQueueListType)));
  connect(m_jsonrpc,
          SIGNAL(successfulSubmissionReceived(mqIdType,mqIdType,mqIdType,QDir)),
          this,
          SLOT(successfulSubmissionReceived(mqIdType,mqIdType,mqIdType,QDir)));
  connect(m_jsonrpc,
          SIGNAL(failedSubmissionReceived(mqIdType,JobSubmissionErrorCode,QString)),
          this,
          SLOT(failedSubmissionReceived(mqIdType,JobSubmissionErrorCode,QString)));
  connect(m_jsonrpc,
          SIGNAL(jobCancellationConfirmationReceived(mqIdType,mqIdType)),
          this,
          SLOT(jobCancellationConfirmationReceived(mqIdType,mqIdType)));
  connect(m_jsonrpc, SIGNAL(jobStateChangeReceived(mqIdType,JobState,JobState)),
          this, SLOT(jobStateChangeReceived(mqIdType,JobState,JobState)));
}

Client::~Client()
{
  delete m_jobArray;
  m_jobArray = NULL;

  delete m_submittedLUT;
  m_submittedLUT = NULL;
}

JobRequest &Client::newJobRequest()
{
  m_jobArray->push_back(JobRequest (this));
  JobRequest &newJob = m_jobArray->back();
  newJob.setClientId(m_jobArray->size());
  return newJob;
}

mqQueueListType Client::queueList() const
{
  return m_queueList;
}

void Client::submitJobRequest(const JobRequest &req)
{
  const mqIdType id = this->nextPacketId();
  const mqPacketType packet = m_jsonrpc->generateJobRequest(req, id);
  m_submittedLUT->insert(id, req.clientId());
  this->sendPacket(packet);
}

void Client::cancelJobRequest(const JobRequest &req)
{
  const mqIdType id = this->nextPacketId();
  const mqPacketType packet = m_jsonrpc->generateJobCancellation(req, id);
  m_canceledLUT->insert(id, req.clientId());
  this->sendPacket(packet);
}

void Client::queueListReceived(mqIdType, const mqQueueListType &list)
{
  m_queueList = list;
  emit queueListUpdated(m_queueList);
}

void Client::successfulSubmissionReceived(mqIdType packetId,
                                          mqIdType moleQueueId,
                                          mqIdType queueJobId,
                                          const QDir &workingDir)
{
  if (!m_submittedLUT->contains(packetId)) {
    qWarning() << "Client received a submission confirmation with an "
                  "unrecognized packet id.";
    return;
  }

  const mqIdType clientId = m_submittedLUT->take(packetId);
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

void Client::failedSubmissionReceived(mqIdType packetId,
                                      JobSubmissionErrorCode errorCode,
                                      const QString &errorMessage)
{
  if (!m_submittedLUT->contains(packetId)) {
    qWarning() << "Client received a submission confirmation with an "
                  "unrecognized packet id.";
    return;
  }

  const mqIdType clientId = m_submittedLUT->take(packetId);
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

void Client::jobCancellationConfirmationReceived(mqIdType packetId,
                                                 mqIdType moleQueueId)
{
  if (!m_submittedLUT->contains(packetId)) {
    qWarning() << "Client received a submission confirmation with an "
                  "unrecognized packet id.";
    return;
  }

  const mqIdType clientId = m_submittedLUT->take(packetId);
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

void Client::jobStateChangeReceived(mqIdType moleQueueId,
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
  mqPacketType packet = m_jsonrpc->generateQueueListRequest(
        this->nextPacketId());
  this->sendPacket(packet);
}

JobRequest * Client::jobRequestByClientId(mqIdType clientId)
{
  if (clientId < 1 || clientId > static_cast<mqIdType>(m_jobArray->size())) {
    qWarning() << "Request for JobRequest with invalid clientId:" << clientId;
    return NULL;
  }
  return &((*m_jobArray)[clientId - 1]);
}

const JobRequest * Client::jobRequestByClientId(mqIdType clientId) const
{
  if (clientId < 1 || clientId > static_cast<mqIdType>(m_jobArray->size())) {
    qWarning() << "Request for JobRequest with invalid clientId:" << clientId;
    return NULL;
  }
  return &((*m_jobArray)[clientId - 1]);
}

JobRequest *Client::jobRequestByMoleQueueId(mqIdType moleQueueId)
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

const JobRequest *Client::jobRequestByMoleQueueId(mqIdType moleQueueId) const
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
