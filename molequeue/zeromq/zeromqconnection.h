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

#ifndef ZEROMQCONNECTION_H_
#define ZEROMQCONNECTION_H_

#include <molequeue/servercore/connection.h>

#include <zmq.hpp>

class QTimer;

namespace MoleQueue
{

/// @brief Connection subclass using ZeroMQ.
class ZeroMqConnection: public MoleQueue::Connection
{
  Q_OBJECT
public:
  ZeroMqConnection(QObject *parentObject,
                   zmq::context_t *context,
                   zmq::socket_t *socket);
  ZeroMqConnection(QObject *parentObject, const QString &address);
  ~ZeroMqConnection();

  /**
   * Open the connection
   */
  void open();

  /**
   * Start receiving messages on this connection
   */
  void start();

  /**
   * Close the connection. Once a conneciton is closed if can't reused.
   */
  void close();

  /**
   * @return true, if the connection is open ( open has been called,
   * false otherwise
   */
  bool isOpen();

  /**
   * @return the connect string description the endpoint the connection is
   * connected to.
   */
  QString connectionString() const;

  bool send(const PacketType &packet, const EndpointIdType &endpoint);

  void flush();

  static const QString zeroMqPrefix;

private slots:
  void listen();

private:
  bool dealerReceive();
  bool routerReceive();

  QString m_connectionString;
  zmq::context_t *m_context;
  zmq::socket_t *m_socket;
  int m_socketType;
  bool m_connected;
  bool m_listening;
};

} /* namespace MoleQueue */

#endif /* ZEROMQCONNECTION_H_ */
