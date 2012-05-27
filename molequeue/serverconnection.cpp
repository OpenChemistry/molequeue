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
#include "jsonrpc.h"

#include <QtCore/QDateTime>
#include <QtCore/QMap>

#include <QtNetwork/QLocalSocket>

#define DEBUGOUT(title) \
  if (this->m_debug)    \
    qDebug() << QDateTime::currentDateTime().toString() \
             << "ServerConnection" << title <<

namespace MoleQueue
{

ServerConnection::ServerConnection(Server *parentServer,
                                   QLocalSocket *theSocket)
  : m_server(parentServer),
    m_holdRequests(true)
{
  qRegisterMetaType<Job>("Job");
  qRegisterMetaType<QueueListType>("QueueListType");

  this->setSocket(theSocket);

  connect(m_jsonrpc, SIGNAL(queueListRequestReceived(IdType)),
          this, SLOT(queueListRequestReceived(IdType)));
  connect(m_jsonrpc, SIGNAL(jobSubmissionRequestReceived(IdType,QVariantHash)),
          this, SLOT(jobSubmissionRequestReceived(IdType,QVariantHash)));
  connect(m_jsonrpc, SIGNAL(jobCancellationRequestReceived(IdType,IdType)),
          this, SLOT(jobCancellationRequestReceived(IdType,IdType)));
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
  this->sendPacket(packet);
}

void ServerConnection::sendSuccessfulSubmissionResponse(const Job &req)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  const IdType clientId = req.clientId();
  if (!m_submissionLUT.contains(clientId)) {
    qWarning() << "Refusing to confirm job submission; unrecognized client id:"
               << clientId;
    return;
  }

  const IdType packetId = m_submissionLUT.take(clientId);
  PacketType packet =  m_jsonrpc->generateJobSubmissionConfirmation(
        req.moleQueueId(), req.queueJobId(), req.localWorkingDirectory(),
        packetId);
  this->sendPacket(packet);
}

void ServerConnection::sendFailedSubmissionResponse(const Job &req,
                                                    JobSubmissionErrorCode ec,
                                                    const QString &errorMessage)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  const IdType clientId = req.clientId();
  if (!m_submissionLUT.contains(clientId)) {
    qWarning() << "Refusing to send job failure; unrecognized client id:"
               << clientId;
    return;
  }

  const IdType packetId = m_submissionLUT.take(clientId);

  PacketType packet =  m_jsonrpc->generateErrorResponse(static_cast<int>(ec),
                                                        errorMessage,
                                                        packetId);
  this->sendPacket(packet);
}

void ServerConnection::sendSuccessfulCancellationResponse(const Job &req)
{
  // Lookup the moleQueueId in the hash so that we can send the correct packetId
  const IdType moleQueueId = req.moleQueueId();
  if (!m_cancellationLUT.contains(moleQueueId)) {
    qWarning() << "Refusing to confirm job cancellation; unrecognized id:"
               << moleQueueId;
    return;
  }

  const IdType packetId = m_cancellationLUT.take(moleQueueId);
  PacketType packet =  m_jsonrpc->generateJobCancellationConfirmation(
        req.moleQueueId(), packetId);
  this->sendPacket(packet);
}

void ServerConnection::sendJobStateChangeNotification(const Job &req,
                                                      JobState oldState,
                                                      JobState newState)
{
  PacketType packet = m_jsonrpc->generateJobStateChangeNotification(
        req.moleQueueId(), oldState, newState);
  this->sendPacket(packet);
}

void ServerConnection::queueListRequestReceived(IdType packetId)
{
  m_listQueuesLUT.push_back(packetId);
  emit queueListRequested();
}

void ServerConnection::jobSubmissionRequestReceived(IdType packetId,
                                                    const QVariantHash &options)
{
  Job req;
  req.setFromHash(options);

  m_submissionLUT.insert(req.clientId(), packetId);

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
  m_holdRequests = false;
  DEBUGOUT("startProcessing") "Started handling requests.";
  while (m_socket->bytesAvailable() != 0) {
    DEBUGOUT("startProcessing") "Flushing request backlog...";
    this->readSocket();
  }
}

void ServerConnection::readSocket()
{
  if (m_holdRequests) {
    DEBUGOUT("readSocket") "Skipping socket read; requests are currently held.";
    return;
  }

  this->AbstractRpcInterface::readSocket();
}

} // end namespace MoleQueue
