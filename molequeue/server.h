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

#include <QtCore/QObject>

#include <QtCore/QList>

#include <QtNetwork/QAbstractSocket> // for SocketError enum

class ServerTest;

class QLocalServer;

namespace MoleQueue
{
class ServerConnection;

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
  virtual ~Server();

  /// Used for unit testing
  friend class ::ServerTest;

signals:

  /**
   * Emitted when a new connection is made with a client.
   * @param conn The ServerConnection with the new client.
   */
  void newConnection(ServerConnection *conn);

  /**
   * Emitted when an error occurs.
   *
   * @param error
   * @param message
   */
  void connectionError(QAbstractSocket::SocketError error,
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

protected slots:

  /**
   * Called when the internal socket server has a new connection ready.
   */
  void newConnectionAvailable();

  /**
   * Called when a client disconnects from the server. This function expects
   * sender() to return a ServerConnection.
   */
  void clientDisconnected();

protected:
  /// List of active connections
  QList<ServerConnection*> m_connections;

  /// The internal local socket server
  QLocalServer *m_server;

  /// Used to change the socket name for unit testing.
  bool m_isTesting;

public:
  /// @param d Enable runtime debugging if true.
  void setDebug(bool d) {m_debug = d;}
  /// @return Whether runtime debugging is enabled.
  bool debug() const {return m_debug;}

protected:
  /// Toggles runtime debugging
  bool m_debug;
};

} // end namespace MoleQueue

Q_DECLARE_METATYPE(QAbstractSocket::SocketError)

#endif // SERVER_H
