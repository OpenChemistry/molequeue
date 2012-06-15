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

#include "serverconnection.h"

#include "job.h"
#include "jobmanager.h"
#include "jsonrpc.h"
#include "server.h"
#include "connection.h"

#include <QtCore/QDateTime>
#include <QtCore/QMap>
#include <QtCore/QDebug>

#define DEBUGOUT(title) \
  if (this->m_debug)    \
    qDebug() << QDateTime::currentDateTime().toString() \
             << "ServerConnection" << title <<

namespace MoleQueue
{

ServerConnection::ServerConnection(Server *parentServer,
                                   Connection *connection)
  : m_server(parentServer)
{
  qRegisterMetaType<Job*>("MoleQueue::Job*");
  qRegisterMetaType<const Job*>("const MoleQueue::Job*");
  qRegisterMetaType<QueueListType>("MoleQueue::QueueListType");

  this->setConnection(connection);

  connect(m_jsonrpc, SIGNAL(queueListRequestReceived(MoleQueue::IdType)),
          this, SLOT(queueListRequestReceived(MoleQueue::IdType)));
  connect(m_jsonrpc, SIGNAL(jobSubmissionRequestReceived(MoleQueue::IdType,QVariantHash)),
          this, SLOT(jobSubmissionRequestReceived(MoleQueue::IdType,QVariantHash)));
  connect(m_jsonrpc, SIGNAL(jobCancellationRequestReceived(MoleQueue::IdType,MoleQueue::IdType)),
          this, SLOT(jobCancellationRequestReceived(MoleQueue::IdType,MoleQueue::IdType)));
  connect(m_connection, SIGNAL(disconnected()),
          this, SIGNAL(disconnected()));
}

ServerConnection::~ServerConnection()
{
}

void ServerConnection::sendQueueList(const QueueListType &queueList)
{
  if (m_listQueuesLUT.isEmpty()) {
    qWarning() << Q_FUNC_INFO << "Refusing to send listQueues reply -- no "
                  "pending requests.";
    return;
  }

  IdType packetId = m_listQueuesLUT.takeFirst();
  PacketType packet = m_jsonrpc->generateQueueList(queueList, packetId);
  m_connection->send(packet);
}

void ServerConnection::sendSuccessfulSubmissionResponse(const Job *req)
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
  m_connection->send(packet);
}

void ServerConnection::sendFailedSubmissionResponse(const Job *req,
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
  m_connection->send(packet);
}

void ServerConnection::sendSuccessfulCancellationResponse(const Job *req)
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
  m_connection->send(packet);
}

void ServerConnection::sendJobStateChangeNotification(const Job *req,
                                                      JobState oldState,
                                                      JobState newState)
{
  PacketType packet = m_jsonrpc->generateJobStateChangeNotification(
        req->moleQueueId(), oldState, newState);
  m_connection->send(packet);
}

void ServerConnection::queueListRequestReceived(IdType packetId)
{
  m_listQueuesLUT.push_back(packetId);
  emit queueListRequested();
}

void ServerConnection::jobSubmissionRequestReceived(IdType packetId,
                                                    const QVariantHash &options)
{
  Job *req = m_server->jobManager()->newJob(options);

  m_submissionLUT.insert(req->moleQueueId(), packetId);
  m_ownedJobMoleQueueIds.append(req->moleQueueId());

  emit jobSubmissionRequested(req);
}

void ServerConnection::jobCancellationRequestReceived(IdType packetId,
                                                      IdType moleQueueId)
{
  m_cancellationLUT.insert(moleQueueId, packetId);
  emit jobCancellationRequested(moleQueueId);
}

void ServerConnection::startProcessing()
{
  m_connection->start();
}

} // end namespace MoleQueue
