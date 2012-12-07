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

#include "zeromqconnection.h"

#include <QtCore/QTimer>
#include <QtCore/QDebug>

namespace MoleQueue
{

const QString ZeroMqConnection::zeroMqPrefix = "zmq";

ZeroMqConnection::ZeroMqConnection(QObject *parentObject,
                                   zmq::context_t *context,
                                   zmq::socket_t *socket)
: Connection(parentObject),
  m_context(context),
  m_socket(socket),
  m_connected(true),
  m_listening(false)
{
  std::size_t socketTypeSize = sizeof(m_socketType);
  m_socket->getsockopt(ZMQ_TYPE, &m_socketType, &socketTypeSize);
}


ZeroMqConnection::ZeroMqConnection(QObject *parentObject, const QString &address)
: Connection(parentObject),
  m_connectionString(address),
  m_context(new zmq::context_t(1)),
  m_socket(new zmq::socket_t(*m_context, ZMQ_DEALER)),
  m_connected(false)
{
  m_socketType = ZMQ_DEALER;
}

ZeroMqConnection::~ZeroMqConnection()
{
  close();
  delete m_context;
  m_context = NULL;
  delete m_socket;
  m_socket = NULL;
}

void ZeroMqConnection::open()
{
  if (m_socket) {
    QByteArray ba = m_connectionString.toLocal8Bit();
    m_socket->connect(ba.data());
    m_connected = true;
  }
}

void ZeroMqConnection::start()
{
  if (!m_listening) {
    m_listening = true;
    QTimer::singleShot(0, this, SLOT(listen()));
  }
}

void ZeroMqConnection::close()
{
  if (m_listening) {
    m_listening = false;
    m_socket->close();
  }
}

bool ZeroMqConnection::isOpen()
{
  return m_connected;
}

QString ZeroMqConnection::connectionString() const
{
  return m_connectionString;
}

void ZeroMqConnection::listen()
{
  if (m_listening) {
    // singleShotTimeout is the time (in ms) until the next call to this
    // function is posted to Qt's event loop. It is 500 ms is there is no
    // activity on the socket, 50 ms if a message was just received, and 0
    // (e.g. immediate) if a message was just received and there are more
    // waiting.
    int singleShotTimeout = 500;
    int rc = 0;

    bool recvd = false;
    if (m_socketType == ZMQ_DEALER)
      recvd = dealerReceive();
    else if (m_socketType == ZMQ_ROUTER)
      recvd = routerReceive();
    else
      qWarning() << "Invalid socket type";

    if (recvd) {
      // Message received -- lower the timeout and poll the socket for more data
      singleShotTimeout = 50;
      zmq::pollitem_t item[1];
      item[0].socket = static_cast<void*>(*m_socket);
      item[0].events = ZMQ_POLLIN;
      try {
        rc = zmq::poll(item, 1, 0);
      }
      catch (zmq::error_t e) {
        qWarning("zmq exception during poll: Error %d: %s", e.num(), e.what());
      }
    }

    // If there was more data, immediately post the next listen event.
    if (rc > 0)
      singleShotTimeout = 0;

    QTimer::singleShot(singleShotTimeout, this, SLOT(listen()));
  }
}

bool ZeroMqConnection::dealerReceive()
{
  zmq::message_t message;
  if(m_socket->recv(&message, ZMQ_NOBLOCK)) {
    int size = message.size();
    PacketType packet(static_cast<char*>(message.data()), size);
    emit packetReceived(packet, EndpointIdType());
    return true;
  }
  return false;
}

bool ZeroMqConnection::routerReceive()
{
  zmq::message_t address;
  if (m_socket->recv(&address, ZMQ_NOBLOCK)) {
    int size = address.size();
    EndpointIdType replyTo(static_cast<char*>(address.data()), size);

    // Now receive the message
    zmq::message_t message;
    if(!m_socket->recv(&message, ZMQ_NOBLOCK)) {
      qWarning() << "Error no message body received";
      return true;
    }

    PacketType packet(static_cast<char*>(message.data()), message.size());

    emit packetReceived(packet, replyTo);

    return true;
  }
  return false;
}

bool MoleQueue::ZeroMqConnection::send(const MoleQueue::PacketType &packet,
                                       const MoleQueue::EndpointIdType &endpoint)
{
  zmq::message_t zmqMessage(packet.size());
  memcpy(zmqMessage.data(), packet.constData(), packet.size());
  bool rc;

  // If on the server side send the endpoint id first
  if (m_socketType == ZMQ_ROUTER) {
    zmq::message_t identity(endpoint.size());
    memcpy(identity.data(), endpoint.data(), endpoint.size());
    try {
     rc  = m_socket->send(identity, ZMQ_SNDMORE | ZMQ_NOBLOCK);
    }
    catch (zmq::error_t e) {
      qWarning("zmq exception during endpoint send: Error %d: %s",
               e.num(), e.what());
      return false;
    }

    if (!rc) {
      qWarning() << "zmq_send failed with EAGAIN";
      return false;
    }
  }

  // Send message body
  try {
    rc = m_socket->send(zmqMessage, ZMQ_NOBLOCK);
  }
  catch (zmq::error_t e) {
    qWarning("zmq exception during message send: Error %d: %s",
             e.num(), e.what());
    return false;
  }

  if (!rc) {
    qWarning() << "zmq_send failed with EAGAIN";
    return false;
  }
  return true;
}

} /* namespace MoleQueue */
