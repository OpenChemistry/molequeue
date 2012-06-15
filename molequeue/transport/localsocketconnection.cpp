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

#include "../abstractrpcinterface.h"
#include "localsocketconnection.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtNetwork/QLocalSocket>

#define DEBUGOUT(title) \
  qDebug() << QDateTime::currentDateTime().toString() \
           << "LocalSocketConnection" << title <<

namespace MoleQueue
{

LocalSocketConnection::LocalSocketConnection(QObject *parentObject,
                                             QLocalSocket *socket)
  : Connection(parentObject),
    m_connectionString(socket->serverName()),
    m_socket(NULL),
    m_headerVersion(1),
    m_headerSize(sizeof(quint32) + sizeof(quint32)),
    m_currentPacketSize(0),
    m_currentPacket(),
    m_dataStream(new QDataStream ()),
    m_holdRequests(true)
{
  this->setSocket(socket);
}

LocalSocketConnection::LocalSocketConnection(QObject *parentObject,
                                             const QString &serverName)
  : Connection(parentObject),
    m_connectionString(serverName),
    m_socket(NULL),
    m_headerVersion(1),
    m_headerSize(sizeof(quint32) + sizeof(quint32)),
    m_currentPacketSize(0),
    m_currentPacket(),
    m_dataStream(new QDataStream ()),
    m_holdRequests(true)
{
  this->setSocket(new QLocalSocket());
}

LocalSocketConnection::~LocalSocketConnection()
{
  // Make sure we are closed
  this->close();

  if (m_socket) {
    delete m_socket;
    m_socket = NULL;
  }

  delete m_dataStream;
  m_dataStream = NULL;

}

void LocalSocketConnection::setSocket(QLocalSocket *socket)
{
  if (m_socket != NULL) {
    m_socket->abort();
    m_socket->disconnect(this);
    this->disconnect(m_socket);
    m_socket->deleteLater();
  }
  if (socket != NULL) {
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(readSocket()));
    connect(socket, SIGNAL(disconnected()),
            this, SIGNAL(disconnected()));
    connect(socket, SIGNAL(destroyed()),
            this, SLOT(socketDestroyed()));
  }
  m_dataStream->setDevice(socket);
  m_socket = socket;
}

void LocalSocketConnection::readSocket()
{
  if (m_holdRequests) {
    DEBUG("readSocket") "Skipping socket read; requests are currently held.";
    return;
  }

  DEBUG("readSocket") "New data available";
  // Check if the data is a new packet or if we're in the middle of reading one.
  if (m_currentPacketSize == 0) {

    // Read header info (e.g. packet size) if enough data is available.
    // Otherwise, wait for more data.
    if (this->canReadPacketHeader())
      m_currentPacketSize = this->readPacketHeader();
    else
      return;
  }

  PacketType block;
  (*m_dataStream) >> block;

  // Add this block to the packet which is in progress
  m_currentPacket.append(block);

  // Are we done?
  if (static_cast<qint64>(m_currentPacket.size()) == m_currentPacketSize) {
    DEBUG("readSocket") "Packet completed. Size:" << m_currentPacketSize;
    emit newMessage(m_currentPacket);
    m_currentPacket.clear();
    m_currentPacketSize = 0;
  }
  else {
    DEBUG("readSocket") "Packet incomplete. Waiting for more data..."
        << "current size:" << m_currentPacket.size() << "bytes of"
        << m_currentPacketSize;
  }
}

void LocalSocketConnection::writePacketHeader(const PacketType &packet)
{
  DEBUG("writePacketHeader") "Writing packet header."
      << "Version:" << m_headerVersion
      << "Size:" << packet.size();

  // First write a version identifier
  (*m_dataStream) << m_headerVersion;

  // Next is the packet size as 32-bit unsigned integer
  (*m_dataStream) << static_cast<quint32>(packet.size());
}

void LocalSocketConnection::send(const PacketType &packet)
{
  DEBUG("sendPacket") "Sending new packet. Size:" << packet.size();
  this->writePacketHeader(packet);
  m_dataStream->writeBytes(packet.constData(),
                           static_cast<unsigned int>(packet.size()));
  m_socket->flush();
}

bool LocalSocketConnection::canReadPacketHeader()
{
  return (m_socket->bytesAvailable() >= m_headerSize);
}

quint32 LocalSocketConnection::readPacketHeader()
{
  Q_ASSERT_X(m_socket->bytesAvailable() >= m_headerSize, Q_FUNC_INFO,
             "Not enough data available to read header! This should not "
             "happen.");

  quint32 headerVersion;
  quint32 packetSize;

  (*m_dataStream) >> headerVersion;

  if (headerVersion != m_headerVersion) {
    qWarning() << "Warning -- MoleQueue client/server version mismatch!";
    return 0;
  }

  (*m_dataStream) >> packetSize;

  DEBUG("readPacketHeader") "Reading packet header."
      << "Version:" << headerVersion
      << "Size:" << packetSize;

  return packetSize;
}

void LocalSocketConnection::open()
{
  if (m_socket) {
    if(isOpen()) {
      qWarning() << "Socket already connected to" << m_connectionString;
      return;
    }

    m_socket->connectToServer(m_connectionString);

    DEBUG("open") "Connected to" << ((m_socket == NULL)
                                   ? QString("Nothing!")
                                   : m_socket->serverName());
  }
}

void LocalSocketConnection::start()
{
  if (m_socket) {
    m_holdRequests = false;
    DEBUG("start") "Started handling requests.";
    while (m_socket->bytesAvailable() != 0) {
      DEBUG("start") "Flushing request backlog...";
      this->readSocket();
    }
  }
}

void LocalSocketConnection::close()
{
  if(m_socket) {
    if(m_socket->isOpen()) {
      m_socket->disconnectFromServer();
      m_socket->close();
    }
  }
}

bool  LocalSocketConnection::isOpen()
{
  return m_socket != NULL && m_socket->isOpen();
}

QString LocalSocketConnection::connectionString()
{
  return m_connectionString;
}

void LocalSocketConnection::socketDestroyed()
{
  // Set to NULL so we know we don't need to clean up
  m_socket = NULL;
  // Tell anyone listening we have been disconnected.
  emit disconnected();
}

} /* namespace MoleQueue */
