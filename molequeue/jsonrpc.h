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

#ifndef JSONRPC_H
#define JSONRPC_H

#include "molequeueglobal.h"

#include <json/json-forwards.h>

#include <QtCore/QHash>
#include <QtCore/QObject>

class QDir;
class QVariant;

namespace MoleQueue
{
class JobRequest;
class QueueManager;


/**
 * @class JsonRpc jsonrpc.h <molequeue/jsonrpc.h>
 * @brief Generate and interpret MoleQueue JSON-RPC packets
 * @author David C. Lonie
 *
 * The JsonRpc class is used to generate and handle JSON-RPC packets that
 * conform to the MoleQueue JSON-RPC specification
 * (http://wiki.openchemistry.org/MoleQueue_JSON-RPC_Specification).
 *
 * This class is used internally by MoleQueueClient and MoleQueueServer objects
 * and should not need to be used directly.
 *
 */
class JsonRpc : public QObject
{
  Q_OBJECT
public:
  /**
    * Constructor.
    *
    * @param parentObject The parent of this instance.
    */
  explicit JsonRpc(QObject *parentObject = 0);

  /**
    * Destructor.
    */
  virtual ~JsonRpc();

  /**
    * Generate a JSON-RPC packet for the job submission request described by
    * @a req.
    *
    * @param req The JobRequest of interest.
    * @param packetId The JSON-RPC id for the request.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    */
  mqPacketType generateJobRequest(const JobRequest &req, mqIdType packetId);

  /**
    * Generate a JSON-RPC packet to confirm a successful job submission.
    *
    * @param moleQueueJobId The MoleQueue internal job identifier
    * @param queueJobId The Queue job id (if applicable, 0 otherwise)
    * @param workingDir Local working directory where files are stored during
    * job execution
    * @param packetId The JSON-RPC id for the request.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    */
  mqPacketType generateJobSubmissionConfirmation(mqIdType moleQueueJobId,
                                                 mqIdType queueJobId,
                                                 const QString &workingDir,
                                                 mqIdType packetId);

  /**
    * Generate a JSON-RPC response packet to notify of an error.
    *
    * @param errorCode Error code
    * @param message Single sentence describing the error that occurred.
    * @param packetId The JSON-RPC id for the packet.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    */
  mqPacketType generateErrorResponse(int errorCode,
                                     const QString &message,
                                     mqIdType packetId);

  /**
    * Generate a JSON-RPC response packet to notify of an error.
    *
    * @param errorCode Error code
    * @param message Single sentence describing the error that occurred.
    * @param data a Json::Value to be used as the error object's data member
    * @param packetId The JSON-RPC id for the packet.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    * @overload
    */
  mqPacketType generateErrorResponse(int errorCode,
                                     const QString &message,
                                     const Json::Value &data,
                                     mqIdType packetId);

  /**
    * Generate a JSON-RPC response packet to notify of an error.
    *
    * @param errorCode Error code
    * @param message Single sentence describing the error that occurred.
    * @param packetId The JSON-RPC id for the packet.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    * @overload
    */
  mqPacketType generateErrorResponse(int errorCode,
                                     const QString &message,
                                     const Json::Value &packetId);

  /**
    * Generate a JSON-RPC response packet to notify of an error.
    *
    * @param errorCode Error code
    * @param message Single sentence describing the error that occurred.
    * @param data a Json::Value to be used as the error object's data member
    * @param packetId The JSON-RPC id for the packet.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    * @overload
    */
  mqPacketType generateErrorResponse(int errorCode,
                                     const QString &message,
                                     const Json::Value &data,
                                     const Json::Value &packetId);

  /**
    * Generate a JSON-RPC packet for requesting a job cancellation.
    *
    * @param req The JobRequest to cancel.
    * @param packetId The JSON-RPC id for the request.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    */
  mqPacketType generateJobCancellation(const JobRequest &req,
                                       mqIdType packetId);

  /**
    * Generate a JSON-RPC packet confirming a job cancellation.
    *
    * @param moleQueueId MoleQueue internal identifer for the canceled job.
    * @param packetId The JSON-RPC id for the request.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    */
  mqPacketType generateJobCancellationConfirmation(mqIdType moleQueueId,
                                                   mqIdType packetId);

  /**
    * Generate a JSON-RPC packet for requesting a list of available Queues and
    * Programs.
    *
    * @param packetId The JSON-RPC id for the request.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    */
  mqPacketType generateQueueListRequest(mqIdType packetId);

  /**
    * Generate a JSON-RPC packet to request a listing of all available Queues
    * and Programs.
    *
    * @param qmanager The QueueManager to send.
    * @param packetId The JSON-RPC id for the request.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    */
  mqPacketType generateQueueList(const QueueManager *qmanager,
                                 mqIdType packetId);

  /**
    * Generate a JSON-RPC packet to notify listeners that a job has changed
    * states.
    *
    * @param moleQueueJobId Internal MoleQueue job id of job.
    * @param oldState Old state of the job.
    * @param newState New state of the job.
    * @return A mqPacketType, ready to send to a QLocalSocket.
    */
  mqPacketType generateJobStateChangeNotification(mqIdType moleQueueJobId,
                                                  JobState oldState,
                                                  JobState newState);

  /**
    * Read a newly received packet.
    * The packet(s) are split and interpreted, and signals are emitted
    * depending on the type of packets.
    *
    * @param data A packet containing a single or batch JSON-RPC transmission.
    * @return A QVector containing the packetIds of the data.
    */
  void interpretIncomingPacket(const mqPacketType &data);

  /**
    * Process a JSON-RPC packet and return a QVector containing the packetId(s).
    * The packet(s) are split and interpreted, and signals are emitted
    * depending on the type of packets.
    *
    * @param data A JsonCpp value containing a single or batch JSON-RPC
    * transmission.
    * @return A QVector containing the packetIds of the data.
    */
  void interpretIncomingJsonRpc(const Json::Value &data);

  /**
    * @param strict If false, minor errors (e.g. extra keys) will result in a
    * warning, but will not cause the function to return false.
    * @return True if the input JSON snippet is a valid JSON-RPC 2.0 request.
    */
  bool validateRequest(const mqPacketType &, bool strict = false);

  /**
    * @param strict If false, minor errors (e.g. extra keys) will result in a
    * warning, but will not cause the function to return false.
    * @return True if the input JSON object is a valid JSON-RPC 2.0 request.
    */
  bool validateRequest(const Json::Value &, bool strict = false);

  /**
    * @param strict If false, minor errors (e.g. extra keys) will result in a
    * warning, but will not cause the function to return false.
    * @return True if the input JSON snippet is a valid JSON-RPC 2.0 response.
    */
  bool validateResponse(const mqPacketType &, bool strict = false);

  /**
    * @param strict If false, minor errors (e.g. extra keys) will result in a
    * warning, but will not cause the function to return false.
    * @return True if the input JSON object is a valid JSON-RPC 2.0 response.
    */
  bool validateResponse(const Json::Value &, bool strict = false);

  /**
    * @param strict If false, minor errors (e.g. extra keys) will result in a
    * warning, but will not cause the function to return false.
    * @return True if the input JSON snippet is a valid JSON-RPC 2.0
    * notification.
    */
  bool validateNotification(const mqPacketType &, bool strict = false);

  /**
    * @param strict If false, minor errors (e.g. extra keys) will result in a
    * warning, but will not cause the function to return false.
    * @return True if the input JSON object is a valid JSON-RPC 2.0
    * notification.
    */
  bool validateNotification(const Json::Value &, bool strict = false);

signals:

  /**
    * Emitted when a packet containing invalid JSON is received. The connected
    * client or server must send an error -32700 "Parse error".
    *
    * @param packetId JSON value representing the packetId
    * @param errorDataObject JSON object to be used as the data value in the
    * error object.
    */
  void invalidPacketReceived(const Json::Value &packetId,
                             const Json::Value &errorDataObject) const;

  /**
    * Emitted when an invalid JSON-RPC request is received. The connected
    * client or server must send an error -32600 "Invalid request".
    *
    * @param packetId JSON value representing the packetId
    * @param errorDataObject JSON object to be used as the data value in the
    * error object.
    */
  void invalidRequestReceived(const Json::Value &packetId,
                              const Json::Value &errorDataObject) const;

  /**
    * Emitted when a valid JSON-RPC request with an unknown method is received.
    * The connected client or server must send an error -32601
    * "Method not found".
    *
    * @param packetId JSON value representing the packetId
    * @param errorDataObject JSON object to be used as the data value in the
    * error object.
    */
  void unrecognizedRequestReceived(const Json::Value &packetId,
                                   const Json::Value &errorDataObject) const;

  /**
    * Emitted when a valid JSON-RPC request with a known method and invalid
    * parameters is received. The connected client or server must send an
    * error -32602 "Invalid params".
    *
    * @param packetId JSON value representing the packetId
    * @param errorDataObject JSON object to be used as the data value in the
    * error object.
    */
  void invalidRequestParamsReceived(const Json::Value &packetId,
                                    const Json::Value &errorDataObject) const;

  /**
    * Emitted when an internal JSON-RPC error occurs. The connected client or
    * server must send an error -32603 "Internal error".
    *
    * @param packetId JSON value representing the packetId
    * @param errorDataObject JSON object to be used as the data value in the
    * error object.
    */
  void internalErrorOccurred(const Json::Value &packetId,
                             const Json::Value &errorDataObject) const;

  /**
    * Emitted when a request for a list of available Queues/Programs is
    * received.
    *
    * @param packetId The JSON-RPC id for the packet
    */
  void queueListRequestReceived(mqIdType packetId) const;

  /**
    * Emitted when a list of available Queues/Programs is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param options List of Queues/Programs
    */
  void queueListReceived(mqIdType packetId, const mqQueueListType &list) const;

  /**
    * Emitted when a request to submit a new job is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param options Options for the job.
    */
  void jobSubmissionRequestReceived(mqIdType packetId,
                                    const QHash<QString, QVariant> &options) const;

  /**
    * Emitted when a response for a successful job submission is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param moleQueueId An internal identifer unique to the new job.
    * @param jobId An identifier used by the queuing system. If the Queue does
    * not used numeric ids or none are available (e.g. local submission may not
    * provide a process id immediately), 0 will be returned.
    * @param workingDir The local directory where the temporary files will be
    * stored.
    */
  void successfulSubmissionReceived(mqIdType packetId, mqIdType moleQueueId,
                                    mqIdType jobId, const QDir &workingDir) const;

  /**
    * Emitted when a response for an unsuccessful job submission is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param errorCode Error code categorizing the error.
    * @param errorMessage Descriptive string identifying the error.
    */
  void failedSubmissionReceived(mqIdType packetId,
                                JobSubmissionErrorCode errorCode,
                                const QString &errorMessage) const;

  /**
    * Emitted when a request to cancel a job is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param moleQueueId The internal MoleQueue identifier for the job to
    * cancel.
    */
  void jobCancellationRequestReceived(mqIdType packetId,
                                      mqIdType moleQueueId) const;

  /**
    * Emitted when a confirmation of job cancellation is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param moleQueueId The internal MoleQueue identifier for the canceled job.
    */
  void jobCancellationConfirmationReceived(mqIdType packetId,
                                           mqIdType moleQueueId) const;

  /**
    * Emitted when a notification that a job has changed state is received.
    *
    * @param moleQueueId The internal MoleQueue identifier for the job.
    * @param oldState The original state of the job
    * @param newState The new state of the job.
    */
  void jobStateChangeReceived(mqIdType moleQueueId,
                              JobState oldState, JobState newState) const;

public slots:
  /// @param b If true, enable debugging output at runtime.
  void setDebug(bool b) {m_debug = b;}

  /// @return Whether runtime debugging is enabled.
  bool debug() {return m_debug;}

protected:
  /// Create and return a new JsonCpp JSON-RPC request.
  /// @param id JSON-RPC id
  static Json::Value generateEmptyRequest(mqIdType id);
  /// Create and return a new JsonCpp JSON-RPC response.
  /// @param id JSON-RPC id
  static Json::Value generateEmptyResponse(mqIdType id);
  /// Create and return a new JsonCpp JSON-RPC error response.
  /// @param id JSON-RPC id
  static Json::Value generateEmptyError(mqIdType id);
  /// Create and return a new JsonCpp JSON-RPC error response.
  /// @param id JSON-RPC id
  static Json::Value generateEmptyError(const Json::Value &id);
  /// Create and return a new JsonCpp JSON-RPC notification.
  /// @param id JSON-RPC id
  static Json::Value generateEmptyNotification();

  /// Enum describing the types of packets that the implementation is aware of.
  enum PacketType {
    INVALID_PACKET = -1,
    REQUEST_PACKET,
    RESULT_PACKET,
    ERROR_PACKET,
    NOTIFICATION_PACKET
  };

  /// Enum describing the known methods
  enum PacketMethod {
    /// Packet is a response to a request originating from a different client.
    IGNORE_METHOD = -3,
    UNRECOGNIZED_METHOD = -2,
    INVALID_METHOD = -1,
    LIST_QUEUES,
    SUBMIT_JOB,
    CANCEL_JOB,
    JOB_STATE_CHANGED
  };

  /// @param root Input JSOC-RPC packet
  /// @return The PacketType of the packet
  PacketType guessPacketType(const Json::Value &root) const;

  /// @param root Input JSOC-RPC packet
  /// @return The PacketMethod of a request/notification
  PacketMethod guessPacketMethod(const Json::Value &root) const;

  /// Extract data and emit signal for unparsable JSON.
  /// @param root Invalid request data
  void handleUnparsablePacket(const mqPacketType &data) const;
  /// Extract data and emit signal for a invalid request.
  /// @param root Invalid request data
  void handleInvalidRequest(const Json::Value &root) const;
  /// Extract data and emit signal for a unrecognized request method.
  /// @param root Invalid request data
  void handleUnrecognizedRequest(const Json::Value &root) const;

  /// Extract data and emit signal for a listQueues request.
  /// @param root Root of request
  void handleListQueuesRequest(const Json::Value &root) const;
  /// Extract data and emit signal for a listQueues result.
  /// @param root Root of request
  void handleListQueuesResult(const Json::Value &root) const;
  /// Extract data and emit signal for a listQueues error.
  /// @param root Root of request
  void handleListQueuesError(const Json::Value &root) const;

  /// Extract data and emit signal for a submitJob request.
  /// @param root Root of request
  void handleSubmitJobRequest(const Json::Value &root) const;
  /// Extract data and emit signal for a submitJob result.
  /// @param root Root of request
  void handleSubmitJobResult(const Json::Value &root) const;
  /// Extract data and emit signal for a submitJob error.
  /// @param root Root of request
  void handleSubmitJobError(const Json::Value &root) const;

  /// Extract data and emit signal for a cancelJob request.
  /// @param root Root of request
  void handleCancelJobRequest(const Json::Value &root) const;
  /// Extract data and emit signal for a cancelJob result.
  /// @param root Root of request
  void handleCancelJobResult(const Json::Value &root) const;
  /// Extract data and emit signal for a cancelJob error.
  /// @param root Root of request
  void handleCancelJobError(const Json::Value &root) const;

  /// Extract data and emit signal for a jobStateChanged notification.
  /// @param root Root of request
  void handleJobStateChangedNotification(const Json::Value &root) const;

  /**
    * Record that a new request has been sent. This is necessary to identify the
    * matching reply. Register the request prior to sending it. This is handled
    * internally with a request is generated.
    *
    * @param packetId JSON-RPC 'id' value
    * @param method PacketMethod value for request.
    */
  void registerRequest(mqIdType packetId, PacketMethod method);

  /**
    * Register that a reply has been received. This removes the request from the
    * container of pending requests.
    *
    * @param packetId JSON-RPC 'id' value
    */
  void registerReply(mqIdType packetId);

  /// Lookup hash for pending requests
  QHash<mqIdType, PacketMethod> m_pendingRequests;

  /// Toggles runtime debugging output
  bool m_debug;

};

} // end namespace MoleQueue

#endif // JSONRPC_H
