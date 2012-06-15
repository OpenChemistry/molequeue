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

#include "molequeueglobal.h"
#include "connectionlistener.h"

#include <QtCore/QObject>
#include <QtCore/QList>

class ServerTest;

class QSettings;

namespace MoleQueue
{
class Job;
class JobManager;
class QueueManager;
class ServerConnection;
class Connection;
class ConnectionListener;

/**
 * @class Server server.h <molequeue/server.h>
 * @brief The Server class handles incoming Client connections and spawns
 * a ServerConnection instance for each.
 * @author David C. Lonie
 */
class Server : public QObject
{
  Q_OBJECT
public:

  /**
   * Constructor.
   *
   * @param parentObject The parent.
   */
  explicit Server(QObject *parentObject = 0);

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

  /// Used for unit testing
  friend class ::ServerTest;

signals:

  /**
   * Emitted when a new connection is made with a client.
   * @param conn The ServerConnection with the new client.
   */
  void newConnection(MoleQueue::ServerConnection *conn);

  /**
   * Emitted when an error occurs.
   *
   * @param error
   * @param message
   */
  void connectionError(MoleQueue::ConnectionListener::Error error,
                       const QString &message);

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
   */
  void stop();

  /**
   * Find the client that owns @a job and send a notification to the client that
   * the JobState has changed.
   * @param job Job of interest.
   * @param oldState Previous state of @a job.
   * @param newState New state of @a job.
   */
  void dispatchJobStateChange(const MoleQueue::Job *job,
                              MoleQueue::JobState oldState,
                              MoleQueue::JobState newState);

protected slots:

  /**
   * Called when a Client requests a list of available queues and programs.
   */
  void queueListRequested();

  /**
   * Called when a Client submits a new job.
   * @param req The new Job request.
   */
  void jobSubmissionRequested(const MoleQueue::Job *req);

  /**
   * @brief Called when a Client requests a job be canceled.
   * @param moleQueueId
   */
  void jobCancellationRequested(MoleQueue::IdType moleQueueId);

  /**
   * Set the MoleQueue Id of a job before it is added to the manager.
   * @param job The new Job.
   */
  void jobAboutToBeAdded(MoleQueue::Job *job);

  /**
   * Called when the internal socket server has a new connection ready.
   */
  void newConnectionAvailable(MoleQueue::Connection *connection);

  /**
   * Called when a client disconnects from the server. This function expects
   * sender() to return a ServerConnection.
   */
  void clientDisconnected();

protected:
  /**
   * Find the ServerConnection that owns the Job with the request MoleQueue id.
   * @param moleQueueId MoleQueue id of Job.
   * @return A pointer to the ServerConnection, or NULL if no active connection
   * found.
   */
  ServerConnection * lookupConnection(IdType moleQueueId);

  /// List of active connections
  QList<ServerConnection*> m_connections;

  /// The connection listener
  ConnectionListener *m_connectionListener;

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

public:
  /// @param d Enable runtime debugging if true.
  void setDebug(bool d) {m_debug = d;}
  /// @return Whether runtime debugging is enabled.
  bool debug() const {return m_debug;}

protected:
  /// Toggles runtime debugging
  bool m_debug;

private:
  void createConnectionListener();
  QString m_serverName;
};

} // end namespace MoleQueue

#endif // SERVER_H
