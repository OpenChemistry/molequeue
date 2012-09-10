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

#ifndef MOLEQUEUE_CLIENTJSONRPC_H
#define MOLEQUEUE_CLIENTJSONRPC_H

#include "jsonrpc.h"

#include "molequeueglobal.h"
#include "transport/message.h"

#include <json/json-forwards.h>

namespace MoleQueue {
class Job;

/**
 * @class JsonRpc jsonrpc.h <molequeue/jsonrpc.h>
 * @brief Generate and interpret client-side MoleQueue JSON-RPC packets.
 * @author David C. Lonie
 *
 * The ClientJsonRpc class is used to generate and handle JSON-RPC packets that
 * conform to the MoleQueue JSON-RPC specification
 * (http://wiki.openchemistry.org/MoleQueue_JSON-RPC_Specification).
 *
 * This class is used internally by MoleQueue::Client and should not need to be
 * used directly.
 *
 */
class ClientJsonRpc : public MoleQueue::JsonRpc
{
  Q_OBJECT
public:
  explicit ClientJsonRpc(QObject *p = 0);
  ~ClientJsonRpc();

  /**
    * Generate a JSON-RPC packet for the job submission request described by
    * @a req.
    *
    * @param req The Job of interest.
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateJobRequest(const Job &req, IdType packetId);

  /**
    * Generate a JSON-RPC packet for requesting a job cancellation.
    *
    * @param req The Job to cancel.
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateJobCancellation(const Job &req,
                                     IdType packetId);

  /**
    * Generate a JSON-RPC packet for requesting a job lookup.
    *
    * @param moleQueueId The MoleQueue id of the job.
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateLookupJobRequest(IdType moleQueueId, IdType packetId);

  /**
    * Generate a JSON-RPC packet for requesting a list of available Queues and
    * Programs.
    *
    * @param packetId The JSON-RPC id for the request.
    * @return A PacketType, ready to send to a Connection.
    */
  PacketType generateQueueListRequest(IdType packetId);

signals:

  /**
    * Emitted when a list of available Queues/Programs is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param options List of Queues/Programs
    */
  void queueListReceived(MoleQueue::IdType packetId,
                         const MoleQueue::QueueListType &list) const;

  /**
    * Emitted when a response for a successful job submission is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param moleQueueId An internal identifer unique to the new job.
    * @param workingDir The local directory where the temporary files will be
    * stored.
    */
  void successfulSubmissionReceived(MoleQueue::IdType packetId,
                                    MoleQueue::IdType moleQueueId,
                                    const QDir &workingDir) const;

  /**
    * Emitted when a response for an unsuccessful job submission is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param errorCode Error code categorizing the error.
    * @param errorMessage Descriptive string identifying the error.
    */
  void failedSubmissionReceived(MoleQueue::IdType packetId,
                                MoleQueue::JobSubmissionErrorCode errorCode,
                                const QString &errorMessage) const;

  /**
    * Emitted when a confirmation of job cancellation is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param moleQueueId The internal MoleQueue identifier for the canceled job.
    */
  void jobCancellationConfirmationReceived(MoleQueue::IdType packetId,
                                           MoleQueue::IdType moleQueueId) const;

  /**
    * Emitted when a successful lookupJob response is received.
    *
    * @param packetId The JSON-RPC id for the packet.
    * @param hash The requested Job information as a QVariantHash.
    */
  void lookupJobResponseReceived(MoleQueue::IdType packetId,
                                 const QVariantHash &hash) const;

  /**
    * Emitted when a failed lookupJob response is received.
    *
    * @param moleQueueId The requested MoleQueue id.
    * @param packetId The JSON-RPC id for the packet.
    */
  void lookupJobErrorReceived(MoleQueue::IdType packetId,
                              MoleQueue::IdType moleQueueId) const;

  /**
    * Emitted when a notification that a job has changed state is received.
    *
    * @param moleQueueId The internal MoleQueue identifier for the job.
    * @param oldState The original state of the job
    * @param newState The new state of the job.
    */
  void jobStateChangeReceived(MoleQueue::IdType moleQueueId,
                              MoleQueue::JobState oldState,
                              MoleQueue::JobState newState) const;

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

  /// Extract data and emit signal for a listQueues result.
  /// @param root Root of request
  void handleListQueuesResult(const Json::Value &root) const;

  /// Extract data and emit signal for a submitJob result.
  /// @param root Root of request
  void handleSubmitJobResult(const Json::Value &root) const;

  /// Extract data and emit signal for a submitJob error.
  /// @param root Root of request
  void handleSubmitJobError(const Json::Value &root) const;

  /// Extract data and emit signal for a cancelJob result.
  /// @param root Root of request
  void handleCancelJobResult(const Json::Value &root) const;

  /// Extract data and emit signal for a cancelJob error.
  /// @param root Root of request
  void handleCancelJobError(const Json::Value &root) const;

  /// Extract data and emit signal for a lookupJob result.
  /// @param root Root of request
  void handleLookupJobResult(const Json::Value &root) const;

  /// Extract data and emit signal for a lookupJob error.
  /// @param root Root of request
  void handleLookupJobError(const Json::Value &root) const;

  /// Extract data and emit signal for a jobStateChanged notification.
  /// @param root Root of request
  void handleJobStateChangedNotification(const Json::Value &root) const;

};

} // namespace MoleQueue

#endif // MOLEQUEUE_CLIENTJSONRPC_H
