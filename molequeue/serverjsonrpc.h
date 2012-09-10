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

#include "jsonrpc.h"

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
                                               IdType packetId);
  /**
    * Generate a JSON-RPC packet confirming a job cancellation.
    *
    * @param moleQueueId MoleQueue internal identifer for the canceled job.
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateJobCancellationConfirmation(IdType moleQueueId,
                                                 IdType packetId);

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
                                       IdType packetId);

  /**
    * Generate a JSON-RPC packet to request a listing of all available Queues
    * and Programs.
    *
    * @param qmanager The QueueManager to send.
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateQueueList(const QueueListType &queueList,
                               IdType packetId);

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

signals:
  /**
    * Emitted when a request for a list of available Queues/Programs is
    * received.
    *
    * @param connection The connection the request was received on
    * @param replyTo The reply to endpoint to identify the client.
    * @param packetId The JSON-RPC id for the packet
    */
  void queueListRequestReceived(MoleQueue::Connection *connection,
                                const MoleQueue::EndpointId replyTo,
                                MoleQueue::IdType packetId) const;

  /**
    * Emitted when a request to submit a new job is received.
    *
    * @param connection The connection the request was received on
    * @param replyTo The reply to endpoint to identify the client.
    * @param packetId The JSON-RPC id for the packet
    * @param options Options for the job.
    */
  void jobSubmissionRequestReceived(MoleQueue::Connection *connection,
                                    const MoleQueue::EndpointId replyTo,
                                    MoleQueue::IdType packetId,
                                    const QVariantHash &options) const;

  /**
    * Emitted when a request to cancel a job is received.
    *
    * @param connection The connection the request was received on
    * @param replyTo The reply to endpoint to identify the client.
    * @param packetId The JSON-RPC id for the packet
    * @param moleQueueId The internal MoleQueue identifier for the job to
    * cancel.
    */
  void jobCancellationRequestReceived(MoleQueue::Connection *connection,
                                      const MoleQueue::EndpointId replyTo,
                                      MoleQueue::IdType packetId,
                                      MoleQueue::IdType moleQueueId) const;

  /**
    * Emitted when a lookupJob request is received.
    *
    * @param connection The connection the request was received on.
    * @param replyTo The reply to endpoint to identify the client.
    * @param packetId The JSON-RPC id for the packet.
    * @param moleQueueId The internal MoleQueue identifier for the requested
    * job.
    */
  void lookupJobRequestReceived(MoleQueue::Connection *connection,
                                const MoleQueue::EndpointId replyTo,
                                MoleQueue::IdType packetId,
                                MoleQueue::IdType moleQueueId) const;

protected:
  /// Known methods used by the client.
  enum MethodType {
    LIST_QUEUES = 0,
    SUBMIT_JOB,
    CANCEL_JOB,
    LOOKUP_JOB,
    JOB_STATE_CHANGED
  };

  /// Reimplemented from base class.
  int mapMethodNameToInt(const QString &methodName) const;

  /// Reimplemented from base class.
  void handlePacket(int method, PacketForm type, Connection *conn,
                    const EndpointId replyTo, const Json::Value &root);

  /// Extract data and emit signal for a listQueues request.
  /// @param root Root of request
  void handleListQueuesRequest(MoleQueue::Connection *connection,
                               const EndpointId replyTo,
                               const Json::Value &root) const;

  /// Extract data and emit signal for a submitJob request.
  /// @param root Root of request
  void handleSubmitJobRequest(MoleQueue::Connection *connection,
                              const EndpointId replyTo,
                              const Json::Value &root) const;

  /// Extract data and emit signal for a cancelJob request.
  /// @param root Root of request
  void handleCancelJobRequest(MoleQueue::Connection *connection,
                              const EndpointId replyTo,
                              const Json::Value &root) const;

  /// Extract data and emit signal for a lookupJob request.
  /// @param root Root of request
  void handleLookupJobRequest(MoleQueue::Connection *connection,
                              const EndpointId replyTo,
                              const Json::Value &root) const;
};

} // namespace MoleQueue

#endif // MOLEQUEUE_SERVERJSONRPC_H
