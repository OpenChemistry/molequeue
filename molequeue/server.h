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

#ifndef SERVER_H
#define SERVER_H

#include "transport/abstractrpcinterface.h"

#include "job.h"
#include "molequeueglobal.h"
#include "transport/connectionlistener.h"
#include "transport/jsonrpc.h"

#include <QtCore/QObject>
#include <QtCore/QList>

class ServerTest;
class ServerConnectionTest;

class QSettings;

namespace MoleQueue
{
class Connection;
class Error;
class Job;
class JobManager;
class QueueManager;
class ServerConnection;
class ServerJsonRpc;

/**
 * @class Server server.h <molequeue/server.h>
 * @brief The Server class handles incoming Client connections and spawns
 * a ServerConnection instance for each.
 * @author David C. Lonie
 *
 * The Server class is the root of the server-side heirarchy. It is responsible
 * for listening for incoming connections and owns the JobManager and
 * QueueManager used to track Job and Queue objects. It also routes client
 * requests to the appropriate server-side objects.
 *
 */
class Server : public AbstractRpcInterface
{
  Q_OBJECT
public:

  /**
   * Constructor.
   *
   * @param parentObject The parent.
   */
  explicit Server(QObject *parentObject = 0, QString serverName_ = "MoleQueue");

  /**
   * Destructor.
   */
  ~Server();

  /**
   * @return A pointer to the Server JobManager.
   */
  JobManager *jobManager() {return m_jobManager;}

  /**
   * @return A pointer to the Server JobManager.
   */
  const JobManager *jobManager() const {return m_jobManager;}

  /**
   * @return A pointer to the Server QueueManager.
   */
  QueueManager *queueManager() {return m_queueManager;}

  /**
   * @return A pointer to the Server QueueManager.
   */
  const QueueManager *queueManager() const {return m_queueManager;}

  /// @param settings QSettings object to write state to.
  void readSettings(QSettings &settings);
  /// @param settings QSettings object to read state from.
  void writeSettings(QSettings &settings) const;

  /// The working directory where running job file are kept.
  QString workingDirectoryBase() const {return m_workingDirectoryBase;}

  /// The string the server uses to listen for connections.
  QString serverName() const { return m_serverName; }

  /// Used for internal lookup structures
  typedef QMap<IdType, MessageIdType> PacketLookupTable;

  /// Used for unit testing
  friend class ::ServerConnectionTest;

  /// Used for unit testing
  friend class ::ServerTest;

signals:

  /**
   * Emitted when an error occurs.
   *
   * @param error
   * @param message
   */
  void connectionError(MoleQueue::ConnectionListener::Error error,
                       const QString &message);

  /**
   * Emitted when the connection is disconnected.
   */
  void disconnected();

  /**
   * Emitted when a non-critical error occurs that the user should be notified
   * of.
   * @param title Title of the error message
   * @param message Details of the error.
   */
  void errorNotification(const QString &title, const QString &message);

public slots:

  /**
   * Start listening for incoming connections.
   *
   * If an error occurs, connectionError will be emitted. If an
   * AddressInUseError occurs on Unix due to a crashed Server that failed to
   * clean up, call forceStart to remove any existing sockets.
   */
  void start();

  /**
   * Start listening for incoming connections, removing any existing socket
   * handles first.
   */
  void forceStart();

  /**
   * Terminate the socket server.
   *
   * @param Server will pass the value of force when stop it connections.
   */
  void stop(bool force);

  /**
     * Terminate the socket server.
     *
     * Same as stop(false)
     */
  void stop();

  /**
   * Find the client that owns @a job and send a notification to the client that
   * the JobState has changed.
   * @param job Job of interest.
   * @param oldState Previous state of @a job.
   * @param newState New state of @a job.
   */
  void dispatchJobStateChange(const MoleQueue::Job &job,
                              MoleQueue::JobState oldState,
                              MoleQueue::JobState newState);

  /**
   * Sends the @a list to the connected client.
   *
   * @param request The request Message
   * @param queueList The queue List
   */
  void sendQueueList(const MoleQueue::Message &request,
                     const MoleQueue::QueueListType &queueList);

  /**
   * Sends a reply to the client informing them that the job submission was
   * successful.
   * @param request The request Message
   * @param job The Job
   */
  void sendSuccessfulSubmissionResponse(const MoleQueue::Message &request,
                                        const MoleQueue::Job &job);

  /**
   * Sends a reply to the client informing them that the job submission failed.
   * @param request The request Message
   * @param ec Error code
   * @param errorMessage Descriptive string
   */
  void sendFailedSubmissionResponse(const MoleQueue::Message &request,
                                    MoleQueue::ErrorCode ec,
                                    const QString &errorMessage);

  /**
   * Sends a reply to the client informing them that the job cancellation was
   * successful.
   * @param request The request Message
   * @param moleQueueId The id of the job being cancelled
   */
  void sendSuccessfulCancellationResponse(const MoleQueue::Message &request,
                                          IdType moleQueueId);

  /**
   * Sends a reply to the client informing them that the job cancellation was
   * unsuccessful.
   * @param request The request Message
   * @param moleQueueId The id of the job being cancelled
   * @param error The ErrorCode describing the failure
   * @param message A descriptive string detailing the failure.
   */
  void sendFailedCancellationResponse(const MoleQueue::Message &request,
                                      MoleQueue::IdType moleQueueId,
                                      MoleQueue::ErrorCode error,
                                      const QString &message);

  /**
   * Sends a requested job details to the client.
   * @param request The request Message
   * @param req The Job
   */
  void sendSuccessfulLookupJobResponse(const MoleQueue::Message &request,
                                       const MoleQueue::Job &req);

  /**
   * Sends a reply to the client informing them that the job lookup failed.
   * @param request The request Message
   * @param moleQueueId The unknown MoleQueue id.
   */
  void sendFailedLookupJobResponse(const MoleQueue::Message &request,
                                   MoleQueue::IdType moleQueueId);

  /**
   * Sends a notification to the connected client informing them that a job
   * has changed status.
   * @param connectionInfo A Message object populated with connection details
   * for routing the message.
   * @param req The Job that changed JobState.
   * @param oldState The old JobState
   * @param newState The new JobState
   */
  void sendJobStateChangeNotification(const MoleQueue::Message &connectionInfo,
                                      const MoleQueue::Job &req,
                                      MoleQueue::JobState oldState,
                                      MoleQueue::JobState newState);

  /**
   * Sends rpcKill reply to the server.
   * @param request The request Message
   * @param success Whether or not the request can be handled.
   */
  void sendRpcKillResponse(const MoleQueue::Message &request, bool success);

protected slots:

  /**
   * Set the MoleQueue Id of a job before it is added to the manager.
   * @param job The new Job.
   */
  void jobAboutToBeAdded(MoleQueue::Job job);

  /**
   * Called when the internal socket server has a new connection ready.
   */
  void newConnectionAvailable(MoleQueue::Connection *connection);

  /**
   * Called when a client disconnects from the server. This function expects
   * sender() to return a ServerConnection.
   */
  void clientDisconnected();

  /**
   * Called when the JsonRpc instance handles a listQueues request.
   */
  void queueListRequestReceived(const MoleQueue::Message &request);

  /**
   * Called when the JsonRpc instance handles a submitJob request.
   * @param options Option hash (see Job::hash())
   */
  void jobSubmissionRequestReceived(const MoleQueue::Message &request,
                                    const QVariantHash &options);

  /**
   * Called when the JsonRpc instance handles a cancelJob request.
   * @param moleQueueId The MoleQueue identifier of the job to cancel.
   */
  void jobCancellationRequestReceived(const MoleQueue::Message &request,
                                      MoleQueue::IdType moleQueueId);

  /**
   * Called when the JsonRpc instance handles a lookupJob request.
   * @param moleQueueId The MoleQueue identifier of the requested job.
   */
  void lookupJobRequestReceived(const MoleQueue::Message &request,
                                MoleQueue::IdType moleQueueId);

  /**
   * Called when the JsonRpc instance handles an rpcKill request.
   */
  void rpcKillRequestReceived(const MoleQueue::Message &request);


private slots:

  /**
   * Called to clean up connection map when a job is removed ...
   */
  void jobRemoved(MoleQueue::IdType moleQueueId);

protected:

  /**
   * @brief timerEvent Reimplemented from QObject.
   */
  void timerEvent(QTimerEvent *);

  /**
   * Find the ServerConnection that owns the Job with the request MoleQueue id.
   * @param moleQueueId MoleQueue id of Job.
   * @return A pointer to the ServerConnection, or NULL if no active connection
   * found.
   */
  ServerConnection * lookupConnection(IdType moleQueueId);

  /// Reimplemented from base class
  void setJsonRpc(JsonRpc *jsonrpc);

  /// Get the internal JsonRpc instance as a ServerJsonRpc object.
  ServerJsonRpc *serverJsonRpc()
  {
    return reinterpret_cast<ServerJsonRpc*>(m_jsonrpc);
  }

  /// List of active connections
  QList<Connection*> m_connections;

  /// The JobManager for this Server.
  JobManager *m_jobManager;

  /// The QueueManager for this Server.
  QueueManager *m_queueManager;

  /// Used to change the socket name for unit testing.
  bool m_isTesting;

  /// Local directory for running jobs.
  QString m_workingDirectoryBase;

  /// Counter for MoleQueue job ids.
  IdType m_moleQueueIdCounter;

  /// Tracks MoleQueue ids belonging to this connection
  QList<IdType> m_ownedJobMoleQueueIds;

  // job id --> connection for notifications.
  QMap<IdType, Connection*> m_connectionLUT;

  // job id --> reply to endpoint for notifications
  QMap<IdType, EndpointIdType> m_endpointLUT;

private:
  void createConnectionListeners();
  QString m_serverName;
  /// The connection listeners
  QList<ConnectionListener*> m_connectionListeners;

  /// The timer id for the job sync event. jobManager()->syncJobState() is
  /// call regularly by this timer.
  int m_jobSyncTimer;
};

} // end namespace MoleQueue

#endif // SERVER_H
