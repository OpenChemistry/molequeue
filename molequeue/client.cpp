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

#include "clientjsonrpc.h"
#include "jobmanager.h"
#include "transport/connection.h"
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QPair>
#include <QtCore/QVector>

namespace MoleQueue
{

Client::Client(QObject *parentObject) :
  AbstractRpcInterface(parentObject),
  m_jobManager(new JobManager(this)),
  m_submittedLUT(new PacketLookupTable ()),
  m_canceledLUT(new PacketLookupTable ()),
  m_connection(NULL)
{
  qRegisterMetaType<JobRequest>("MoleQueue::JobRequest");
  qRegisterMetaType<JobState>("MoleQueue::JobState");
  qRegisterMetaType<QueueListType>("MoleQueue::QueueListType");

  setJsonRpc(new ClientJsonRpc(this));
}

Client::~Client()
{
  delete m_submittedLUT;
  m_submittedLUT = NULL;

  delete m_canceledLUT;
  m_canceledLUT = NULL;
}

QueueListType Client::queueList() const
{
  return m_queueList;
}

void Client::submitJobRequest(const JobRequest &req)
{
  const IdType id = nextPacketId();
  const PacketType packet = clientJsonRpc()->generateJobRequest(req, id);
  m_submittedLUT->insert(id, req);
  m_connection->send(packet);
}

void Client::cancelJob(const JobRequest &req)
{
  const IdType id = nextPacketId();
  const PacketType packet = clientJsonRpc()->generateJobCancellation(req, id);
  m_canceledLUT->insert(id, req);
  m_connection->send(packet);
}

void Client::lookupJob(IdType moleQueueId)
{
  const IdType id = nextPacketId();
  const PacketType packet =
      clientJsonRpc()->generateLookupJobRequest(moleQueueId,id);
  m_connection->send(packet);
}

void Client::queueListReceived(IdType, const QueueListType &list)
{
  m_queueList = list;
  emit queueListUpdated(m_queueList);
}

void Client::successfulSubmissionReceived(IdType packetId,
                                          IdType moleQueueId,
                                          const QDir &workingDir)
{
  if (!m_submittedLUT->contains(packetId)) {
    qWarning() << "Client received a submission confirmation with an "
                  "unrecognized packet id.";
    return;
  }

  // Need a Job instead of a JobRequest so that we can update variables that
  // JobRequest can't set.
  Job req = Job(m_submittedLUT->take(packetId));
  if (!req.isValid()) {
    qWarning() << "Client received a successful job submission response for a "
                  "job that does not exist in the job list.";
    return;
  }
  req.setMoleQueueId(moleQueueId);
  req.setLocalWorkingDirectory(workingDir.absolutePath());
  if (req.outputDirectory().isEmpty())
    req.setOutputDirectory(req.localWorkingDirectory());
  m_jobManager->moleQueueIdChanged(req);

  emit jobSubmitted(JobRequest(req), true, QString());
}

void Client::failedSubmissionReceived(IdType packetId,
                                      ErrorCode,
                                      const QString &errorMessage)
{
  if (!m_submittedLUT->contains(packetId)) {
    qWarning() << "Client received a submission confirmation with an "
                  "unrecognized packet id.";
    return;
  }

  JobRequest req = m_submittedLUT->take(packetId);
  if (!req.isValid()) {
    qWarning() << "Client received a failed job submission response for a "
                  "job that does not exist in the job list.";
    return;
  }

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

  JobRequest req = m_canceledLUT->take(packetId);
  if (!req.isValid()) {
    qWarning() << "Client received a successful job cancellation response for a "
                  "job that does not exist in the job list.";
    return;
  }

  if (req.moleQueueId() != moleQueueId) {
    qWarning() << "Warning: MoleQueue id of canceled job does not match packet"
                  " id.";
  }

  emit jobCanceled(req, true, QString());
}

void Client::jobCancellationErrorReceived(IdType packetId, IdType moleQueueId,
                                          ErrorCode,
                                          const QString &message)
{
  if (!m_canceledLUT->contains(packetId)) {
    qWarning() << "Client received a submission confirmation with an "
                  "unrecognized packet id.";
    return;
  }

  JobRequest req = m_canceledLUT->take(packetId);
  if (!req.isValid()) {
    qWarning() << "Client received a successful job cancellation response for a "
                  "job that does not exist in the job list.";
    return;
  }

  if (req.moleQueueId() != moleQueueId) {
    qWarning() << "Warning: MoleQueue id of canceled job does not match packet"
                  " id.";
  }

  emit jobCanceled(req, false, message);
}

void Client::lookupJobResponseReceived(IdType,
                                       const QVariantHash &hash)
{
  IdType moleQueueId =
      static_cast<IdType>(hash.value("moleQueueId", InvalidId).toULongLong());
  if (moleQueueId == InvalidId) {
    qWarning() << "Client received a lookup confirmation without a valid"
                  "MoleQueue Id.";
    return;
  }


  Job job = m_jobManager->lookupJobByMoleQueueId(moleQueueId);
  if (job.isValid())  {
    job.setFromHash(hash);
  }
  else {
    job = m_jobManager->newJob(hash);
    job.setMoleQueueId(moleQueueId);
  }

  emit lookupJobComplete(JobRequest(job), moleQueueId);
}

void Client::lookupJobErrorReceived(IdType, IdType moleQueueId)
{
  if (moleQueueId == InvalidId) {
    qWarning() << "Client received a lookup failure notice with an "
                  "invalid molequeue id.";
    return;
  }

  emit lookupJobComplete(JobRequest(), moleQueueId);
}

void Client::jobStateChangeReceived(IdType moleQueueId,
                                    JobState oldState, JobState newState)
{
  // Need a Job here, JobRequest can't update JobState
  Job req = m_jobManager->lookupJobByMoleQueueId(moleQueueId);
  if (!req.isValid()) {
    qWarning() << "Client received a job state change notification for a "
                  "job with an unrecognized MoleQueue id:" << moleQueueId;
    return;
  }

  req.setJobState(newState);

  emit jobStateChanged(JobRequest(req), oldState, newState);
}

void Client::setJsonRpc(JsonRpc *jsonrpc)
{
  AbstractRpcInterface::setJsonRpc(jsonrpc);
  connect(clientJsonRpc(), SIGNAL(queueListReceived(MoleQueue::IdType,
                                                    MoleQueue::QueueListType)),
          this, SLOT(queueListReceived(MoleQueue::IdType,
                                       MoleQueue::QueueListType)));
  connect(clientJsonRpc(), SIGNAL(successfulSubmissionReceived(MoleQueue::IdType,
                                                               MoleQueue::IdType,
                                                               QDir)),
          this, SLOT(successfulSubmissionReceived(MoleQueue::IdType,
                                                  MoleQueue::IdType,
                                                  QDir)));
  connect(clientJsonRpc(),
          SIGNAL(failedSubmissionReceived(MoleQueue::IdType,
                                          MoleQueue::ErrorCode,
                                          QString)),
          this, SLOT(failedSubmissionReceived(MoleQueue::IdType,
                                              MoleQueue::ErrorCode,
                                              QString)));
  connect(clientJsonRpc(),
          SIGNAL(jobCancellationConfirmationReceived(MoleQueue::IdType,
                                                     MoleQueue::IdType)),
          this, SLOT(jobCancellationConfirmationReceived(MoleQueue::IdType,
                                                         MoleQueue::IdType)));
  connect(m_jsonrpc,
          SIGNAL(jobCancellationErrorReceived(MoleQueue::IdType,
                                              MoleQueue::IdType,
                                              MoleQueue::ErrorCode,QString)),
          this, SLOT(jobCancellationErrorReceived(MoleQueue::IdType,
                                                  MoleQueue::IdType,
                                                  MoleQueue::ErrorCode,
                                                  QString)));
  connect(clientJsonRpc(), SIGNAL(lookupJobResponseReceived(MoleQueue::IdType,
                                                            QVariantHash)),
          this, SLOT(lookupJobResponseReceived(MoleQueue::IdType,
                                               QVariantHash)));
  connect(clientJsonRpc(), SIGNAL(lookupJobErrorReceived(MoleQueue::IdType,
                                                         MoleQueue::IdType)),
          this, SLOT(lookupJobErrorReceived(MoleQueue::IdType,
                                            MoleQueue::IdType)));
  connect(clientJsonRpc(), SIGNAL(jobStateChangeReceived(MoleQueue::IdType,
                                                         MoleQueue::JobState,
                                                         MoleQueue::JobState)),
          this, SLOT(jobStateChangeReceived(MoleQueue::IdType,
                                            MoleQueue::JobState,
                                            MoleQueue::JobState)));
}

void Client::requestQueueListUpdate()
{
  PacketType packet = clientJsonRpc()->generateQueueListRequest(nextPacketId());
  m_connection->send(packet);
}

JobRequest Client::newJobRequest()
{
  return m_jobManager->newJob();
}


void Client::setConnection(Connection *connection)
{
  m_connection = connection;

  connect(connection, SIGNAL(newMessage(const MoleQueue::Message)),
          this, SLOT(readPacket(const MoleQueue::Message)));
}

}
