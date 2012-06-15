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

namespace MoleQueue
{
class Job;
class QueueManager;
class Server;
class Connection;

/** This class will be removed during the transport refactor. Not documenting.*/
class ServerConnection : public AbstractRpcInterface
{
  Q_OBJECT
public:

  /**
   * Constructor.
   *
   * @param parentObject parent
   */
  explicit ServerConnection(Server *parentServer, Connection *conn);

  /**
   * Destructor.
   */
  virtual ~ServerConnection();

  /**
   * Test whether the Job with @a moleQueueId originated from this connection.
   * @param moleQueueId MoleQueue Id of Job
   * @return true if the Job belongs to this ServerConnection, false otherwise.
   */
  bool hasJob(IdType moleQueueId) const
  {
    return m_ownedJobMoleQueueIds.contains(moleQueueId);
  }

  /**
   * @return A list of MoleQueue ids indicating which Jobs belong to this
   * connection.
   */
  QList<IdType> ownedJobs() const {return m_ownedJobMoleQueueIds;}

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
  void jobSubmissionRequested(const MoleQueue::Job &req);

  /**
   * Emitted when the client sends a request to cancel a submitted job.
   * @param moleQueueId MoleQueue identifier
   */
  void jobCancellationRequested(MoleQueue::IdType moleQueueId);

  /**
   * Emitted when the connection is disconnected.
   */
  void disconnected();

public slots:

  /**
   * Sends the @a list to the connected client.
   * @param manager The QueueManager List
   * @sa QueueManager::toQueueList()
   */
  void sendQueueList(const MoleQueue::QueueListType &queueList);

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
  void sendFailedSubmissionResponse(const MoleQueue::Job &req,
                                    MoleQueue::JobSubmissionErrorCode ec,
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
  void sendJobStateChangeNotification(const MoleQueue::Job &req,
                                      MoleQueue::JobState oldState,
                                      MoleQueue::JobState newState);

protected slots:

  /**
   * Called when the JsonRpc instance handles a listQueues request.
   */
  void queueListRequestReceived(MoleQueue::IdType);

  /**
   * Called when the JsonRpc instance handles a submitJob request.
   * @param options Option hash (see Job::hash())
   */
  void jobSubmissionRequestReceived(MoleQueue::IdType,
                                    const QVariantHash &options);

  /**
   * Called when the JsonRpc instance handles a cancelJob request.
   * @param moleQueueId The MoleQueue identifier of the job to cancel.
   */
  void jobCancellationRequestReceived(MoleQueue::IdType,
                                      MoleQueue::IdType moleQueueId);

  /**
   * Start handling incoming request. This should be called by the parent server
   * after connections are in place. This function will enable request
   * processing and then flush any pending requests.
   */
  void startProcessing();

protected:
  /// The parent server instance
  Server *m_server;

  /// Tracks MoleQueue ids belonging to this connection
  QList<IdType> m_ownedJobMoleQueueIds;

  /// Tracks queue list requests
  QList<IdType> m_listQueuesLUT;

  /// Tracks job submission requests: moleQueueId --> packetId
  PacketLookupTable m_submissionLUT;

  /// Tracks job cancellation requests: moleQueueId --> packetId
  PacketLookupTable m_cancellationLUT;
};

} // end namespace MoleQueue

#endif // SERVERCONNECTION_H
