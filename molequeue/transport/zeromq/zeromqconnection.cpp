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

void ZeroMqConnection::send(const Message &msg)
{
  zmq::message_t message(msg.data().size());
  memcpy(message.data(), msg.data().constData(), msg.data().size());
  bool rc;

  // If on the server side send the endpoint id first
  if (m_socketType == ZMQ_ROUTER) {
    zmq::message_t identity(msg.endpoint().size());
    memcpy(identity.data(), msg.endpoint().data(), msg.endpoint().size());
    try {
     rc  = m_socket->send(identity, ZMQ_SNDMORE | ZMQ_NOBLOCK);
    }
    catch (zmq::error_t e) {
      qWarning("zmq exception during endpoint send: Error %d: %s",
               e.num(), e.what());
      return;
    }

    if (!rc) {
      qWarning() << "zmq_send failed with EAGAIN";
      return;
    }
  }

  // Send message body
  try {
    rc = m_socket->send(message, ZMQ_NOBLOCK);
  }
  catch (zmq::error_t e) {
    qWarning("zmq exception during message send: Error %d: %s",
             e.num(), e.what());
    return;
  }

  if (!rc) {
    qWarning() << "zmq_send failed with EAGAIN";
  }
}

void ZeroMqConnection::listen()
{
  // time in ms until next call to listen. If there is a packet to read, this
  // will be shortened to 0 (i.e. immediately added to event loop)
  int singleShotTime = 50;
  if (m_socketType == ZMQ_DEALER) {
    if (dealerReceive())
      singleShotTime = 0;
  }
  else if (m_socketType == ZMQ_ROUTER) {
    if (routerReceive())
      singleShotTime = 0;
  }
  else {
    qWarning() << "Invalid socket type";
  }

  if (m_listening)
    QTimer::singleShot(singleShotTime, this, SLOT(listen()));
}

bool ZeroMqConnection::dealerReceive()
{
  zmq::message_t message;
  if(m_socket->recv(&message, ZMQ_NOBLOCK)) {
    int size = message.size();
    PacketType messageBuffer(static_cast<char*>(message.data()), size);
    Message msg(this, EndpointIdType(), messageBuffer);

    emit newMessage(msg);
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

    size = message.size();
    PacketType packet(static_cast<char*>(message.data()), size);

    Message msg(this, replyTo, packet);

    emit newMessage(msg);
    return true;
  }
  return false;
}

} /* namespace MoleQueue */
