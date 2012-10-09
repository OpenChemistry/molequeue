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

#ifndef MOLEQUEUE_SERVERJSONRPC_H
#define MOLEQUEUE_SERVERJSONRPC_H

#include "transport/jsonrpc.h"

#include "molequeueglobal.h"
#include "transport/message.h"

#include <json/json-forwards.h>

namespace MoleQueue {
class Job;

class ServerJsonRpc : public MoleQueue::JsonRpc
{
  Q_OBJECT
public:
  explicit ServerJsonRpc(QObject *p = 0);
  ~ServerJsonRpc();

  /**
    * Generate a JSON-RPC packet to confirm a successful job submission.
    *
    * @param moleQueueId The MoleQueue internal job identifier
    * @param workingDir Local working directory where files are stored during
    * job execution
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateJobSubmissionConfirmation(IdType moleQueueId,
                                               const QString &workingDir,
                                               const MessageIdType &packetId);
  /**
    * Generate a JSON-RPC packet confirming a job cancellation.
    *
    * @param moleQueueId MoleQueue internal identifer for the canceled job.
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateJobCancellationConfirmation(IdType moleQueueId,
                                                 const MessageIdType &packetId);

  /**
    * Generate a JSON-RPC packet indicating a job cancellation error.
    *
    * @param errorCode Error code
    * @param message Descriptive string describing error.
    * @param moleQueueId MoleQueue internal identifer for the canceled job.
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateJobCancellationError(
      MoleQueue::ErrorCode errorCode, const QString &message,
      IdType moleQueueId, const MessageIdType &packetId);

  /**
    * Generate a JSON-RPC packet to respond to a lookupJob request. If the Job
    * is invalid, an error response will be generated.
    *
    * @param req The requested Job.
    * @param moleQueueId MoleQueue id of requested job.
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateLookupJobResponse(const Job &req, IdType moleQueueId,
                                       const MessageIdType &packetId);

  /**
    * Generate a JSON-RPC packet to request a listing of all available Queues
    * and Programs.
    *
    * @param qmanager The QueueManager to send.
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateQueueList(const QueueListType &queueList,
                               const MessageIdType &packetId);

  /**
    * Generate a JSON-RPC packet to notify listeners that a job has changed
    * states.
    *
    * @param moleQueueId Internal MoleQueue job id of job.
    * @param oldState Old state of the job.
    * @param newState New state of the job.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateJobStateChangeNotification(IdType moleQueueId,
                                                JobState oldState,
                                                JobState newState);

  /**
    * Generate a JSON-RPC packet to respond to a rpcKill request.
    *
    * rpcKill is a server-side option, enabled via a command-line option to the
    * molequeue executable. It allows a request with an "rpcKill" method to
    * shutdown the MoleQueue application. This is only intended for testing.
    *
    * @param success If true, RpcKill is enabled in the server and the app will
    * shut down after the reply is sent.
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateRpcKillResponse(bool success,
                                     const MessageIdType &packetId);


signals:

  /**
    * Emitted when a request for a list of available Queues/Programs is
    * received.
    *
    * @param request The request Message object.
    */
  void queueListRequestReceived(const MoleQueue::Message &request) const;

  /**
    * Emitted when a request to submit a new job is received.
    *
    * @param request The request Message object.
    * @param options Options for the job.
    */
  void jobSubmissionRequestReceived(const MoleQueue::Message &request,
                                    const QVariantHash &options) const;

  /**
    * Emitted when a request to cancel a job is received.
    *
    * @param request The request Message object.
    * @param moleQueueId The internal MoleQueue identifier for the job to
    * cancel.
    */
  void jobCancellationRequestReceived(const MoleQueue::Message &request,
                                      MoleQueue::IdType moleQueueId) const;

  /**
    * Emitted when a lookupJob request is received.
    *
    * @param request The request Message object.
    * @param moleQueueId The internal MoleQueue identifier for the requested
    * job.
    */
  void lookupJobRequestReceived(const MoleQueue::Message &request,
                                MoleQueue::IdType moleQueueId) const;

  /**
    * Emitted when an rpcKill request is received.
    *
    * @param request The request Message object.
    */
  void rpcKillRequestReceived(const MoleQueue::Message &request) const;

protected:
  /// Known methods used by the client.
  enum MethodType {
    LIST_QUEUES = 0,
    SUBMIT_JOB,
    CANCEL_JOB,
    LOOKUP_JOB,
    JOB_STATE_CHANGED,
    RPC_KILL
  };

  /// Reimplemented from base class.
  int mapMethodNameToInt(const QString &methodName) const;

  /// Reimplemented from base class.
  void handleMessage(int method, const Message &msg);

  /// Extract data and emit signal for a listQueues request.
  void handleListQueuesRequest(const Message &msg) const;

  /// Extract data and emit signal for a submitJob request.
  void handleSubmitJobRequest(const Message &msg) const;

  /// Extract data and emit signal for a cancelJob request.
  void handleCancelJobRequest(const Message &msg) const;

  /// Extract data and emit signal for a lookupJob request.
  void handleLookupJobRequest(const Message &msg) const;

  /// Extract data and emit signal for an rpcKill request.
  void handleRpcKillRequest(const Message &msg) const;
};

} // namespace MoleQueue

#endif // MOLEQUEUE_SERVERJSONRPC_H
