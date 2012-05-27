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

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include "abstractrpcinterface.h"

#include "molequeueglobal.h"

#include <QtCore/QVariantHash>

class ServerConnectionTest;

class QLocalSocket;

namespace MoleQueue
{
class Job;
class QueueManager;
class Server;

class ServerConnection : public AbstractRpcInterface
{
  Q_OBJECT
public:

  /**
   * Constructor.
   *
   * @param parentObject parent
   */
  explicit ServerConnection(Server *parentServer, QLocalSocket *theSocket);

  /**
   * Destructor.
   */
  virtual ~ServerConnection();

  /// Used for internal lookup structures
  typedef QMap<IdType, IdType> PacketLookupTable;

  /// The Server parent can work on some internal methods
  friend class MoleQueue::Server;

  /// Used for unit testing
  friend class ::ServerConnectionTest;

signals:

  /**
   * Emitted when the client sends a request for the available queues
   * and programs.
   */
  void queueListRequested();

  /**
   * Emitted when the client sends a request for a new job submission.
   * @param req The Job
   */
  void jobSubmissionRequested(const Job &req);

  /**
   * Emitted when the client sends a request to cancel a submitted job.
   * @param moleQueueId MoleQueue identifier
   */
  void jobCancellationRequested(IdType moleQueueId);

public slots:

  /**
   * Sends the @a list to the connected client.
   * @param manager The QueueManager List
   * @sa QueueManager::toQueueList()
   */
  void sendQueueList(const QueueListType &queueList);

  /**
   * Sends a reply to the client informing them that the job submission was
   * successful.
   * @param req The Job
   */
  void sendSuccessfulSubmissionResponse(const Job &req);

  /**
   * Sends a reply to the client informing them that the job submission failed.
   * @param req The Job
   * @param ec Error code
   * @param errorMessage Descriptive string
   */
  void sendFailedSubmissionResponse(const Job &req,
                                    JobSubmissionErrorCode ec,
                                    const QString &errorMessage);

  /**
   * Sends a reply to the client informing them that the job cancellation was
   * successful.
   * @param req The Job
   */
  void sendSuccessfulCancellationResponse(const Job &req);

  /**
   * Sends a notification to the connected client informing them that a job
   * has changed status.
   * @param req
   * @param oldState
   * @param newState
   */
  void sendJobStateChangeNotification(const Job &req,
                                      JobState oldState, JobState newState);

protected slots:

  /**
   * Called when the JsonRpc instance handles a listQueues request.
   */
  void queueListRequestReceived(IdType);

  /**
   * Called when the JsonRpc instance handles a submitJob request.
   * @param options Option hash (see Job::hash())
   */
  void jobSubmissionRequestReceived(IdType, const QVariantHash &options);

  /**
   * Called when the JsonRpc instance handles a cancelJob request.
   * @param moleQueueId The MoleQueue identifier of the job to cancel.
   */
  void jobCancellationRequestReceived(IdType, IdType moleQueueId);

  /**
   * Start handling incoming request. This should be called by the parent server
   * after connections are in place. This function will enable request
   * processing and then flush any pending requests.
   */
  void startProcessing();

  /**
   * Reimplemented from AbstractRpcInterface to respect m_holdRequests.
   */
  void readSocket();

protected:
  /// The parent server instance
  Server *m_server;

  /// Tracks queue list requests
  QList<IdType> m_listQueuesLUT;

  /// Tracks job submission requests: clientId --> packetId
  PacketLookupTable m_submissionLUT;

  /// Tracks job cancellation requests: moleQueueId --> packetId
  PacketLookupTable m_cancellationLUT;

  /// If true, do not read incoming packets from the socket. This is to let
  /// the parent server create connections prior to processing requests.
  /// @sa startProcessing
  bool m_holdRequests;

};

} // end namespace MoleQueue

#endif // SERVERCONNECTION_H
