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

#include "thirdparty/jsoncpp/json/json-forwards.h"

#include <QtCore/QObject>
#include <QtCore/QtContainerFwd> // forward declarations of templated containers

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
    * Read a packet received and return a QVector containing the packetId(s).
    * The packet(s) are split and interpreted, and signals are emitted
    * depending on the type of packets.
    *
    * @param data A packet containing a single or batch JSON-RPC transmission.
    * @return A QVector containing the packetIds of the data.
    */
  QVector<mqIdType> interpretIncomingPacket(const mqPacketType &data);

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
    * Emitted when a request for a list of available Queues/Programs is
    * received.
    *
    * @param packetId The JSON-RPC id for the packet
    */
  void queueListRequestReceived(mqIdType packetId);

  /**
    * Emitted when a list of available Queues/Programs is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param options List of Queues/Programs
    */
  void queueListReceived(mqIdType packetId,
                         const QList<QPair<QString, QStringList> > &list);

  /**
    * Emitted when a request to submit a new job is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param options Options for the job.
    */
  void jobSubmissionRequestReceived(mqIdType packetId,
                                    const QHash<QString, QVariant> &options);

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
                                    mqIdType jobId, const QDir &workingDir);

  /**
    * Emitted when a response for an unsuccessful job submission is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param errorCode Error code categorizing the error.
    * @param errorMessage Descriptive string identifying the error.
    */
  void failedSubmissionReceived(mqIdType packetId,
                                JobSubmissionErrorCode errorCode,
                                const QString &errorMessage);

  /**
    * Emitted when a request to cancel a job is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param moleQueueId The internal MoleQueue identifier for the job to
    * cancel.
    */
  void jobCancellationRequestReceived(mqIdType packetId,
                                      mqIdType moleQueueId);

  /**
    * Emitted when a confirmation of job cancellation is received.
    *
    * @param packetId The JSON-RPC id for the packet
    * @param moleQueueId The internal MoleQueue identifier for the canceled job.
    */
  void jobCancellationConfirmationReceived(mqIdType packetId,
                                           mqIdType moleQueueId);

  /**
    * Emitted when a notification that a job has changed state is received.
    *
    * @param moleQueueId The internal MoleQueue identifier for the job.
    * @param oldState The original state of the job
    * @param newState The new state of the job.
    */
  void jobStateChangeReceived(mqIdType moleQueueId,
                              JobState oldState, JobState newState);

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
  /// Create and return a new JsonCpp JSON-RPC notification.
  /// @param id JSON-RPC id
  static Json::Value generateEmptyNotification();

  bool m_debug;

};

} // end namespace MoleQueue

#endif // JSONRPC_H
