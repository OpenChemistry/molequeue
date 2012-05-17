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

#include "abstractrpcinterface.h"

#include "jsonrpc.h"

#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>

#include <QtNetwork/QLocalSocket>

#include <QtGlobal>

#define DEBUGOUT(title) \
  if (this->m_debug)    \
    qDebug() << QDateTime::currentDateTime().toString() \
             << "AbstractRpcInterface" << title <<

namespace MoleQueue
{

AbstractRpcInterface::AbstractRpcInterface(QObject *parentObject) :
  QObject(parentObject),
  m_headerVersion(1),
  m_headerSize(sizeof(quint32) + sizeof(quint32)),
  m_currentPacketSize(0),
  m_currentPacket(),
  m_socket(new QLocalSocket (this)),
  m_dataStream(new QDataStream (m_socket)),
  m_jsonrpc(new JsonRpc (this)),
  m_packetCounter(0),
  m_debug(false)
{
  m_dataStream->setVersion(QDataStream::Qt_4_7);

  // Randomize the packet counter's starting value.
  qsrand(QDateTime::currentMSecsSinceEpoch());
  m_packetCounter = static_cast<IdType>(qrand());

  connect(m_socket, SIGNAL(readyRead()), this, SLOT(readSocket()));

  connect(m_jsonrpc, SIGNAL(invalidPacketReceived(Json::Value,Json::Value)),
          this, SLOT(replyToInvalidPacket(Json::Value,Json::Value)));
  connect(m_jsonrpc, SIGNAL(invalidRequestReceived(Json::Value,Json::Value)),
          this, SLOT(replyToInvalidRequest(Json::Value,Json::Value)));
  connect(m_jsonrpc, SIGNAL(unrecognizedRequestReceived(Json::Value,Json::Value)),
          this, SLOT(replyToUnrecognizedRequest(Json::Value,Json::Value)));
  connect(m_jsonrpc, SIGNAL(invalidRequestParamsReceived(Json::Value,Json::Value)),
          this, SLOT(replyToinvalidRequestParams(Json::Value,Json::Value)));
  connect(m_jsonrpc, SIGNAL(internalErrorOccurred(Json::Value,Json::Value)),
          this, SLOT(replyWithInternalError(Json::Value,Json::Value)));

  connect(this, SIGNAL(newPacketReady(PacketType)),
          this, SLOT(readPacket(PacketType)));

  m_socket->connectToServer("MoleQueue");
  qDebug() << "Connected to" << m_socket->serverName();
}

AbstractRpcInterface::~AbstractRpcInterface()
{
  m_socket->abort();
  delete m_socket;
  m_socket = NULL;

  delete m_dataStream;
  m_dataStream = NULL;

  delete m_jsonrpc;
  m_jsonrpc = NULL;
}

void AbstractRpcInterface::readSocket()
{
  DEBUGOUT("readSocket") "New data available";
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
    DEBUGOUT("readSocket") "Packet completed. Size:" << m_currentPacketSize;
    emit newPacketReady(m_currentPacket);
    m_currentPacket.clear();
    m_currentPacketSize = 0;
  }
  else {
    DEBUGOUT("readSocket") "Packet incomplete. Waiting for more data..."
        << "current size:" << m_currentPacket.size() << "bytes of"
        << m_currentPacketSize;
  }
}

void AbstractRpcInterface::readPacket(const PacketType &packet)
{
  DEBUGOUT("readPacket") "Interpreting new packet.";
  m_jsonrpc->interpretIncomingPacket(packet);
}

void AbstractRpcInterface::sendPacket(const PacketType &packet)
{
  DEBUGOUT("sendPacket") "Sending new packet. Size:" << packet.size();
  this->writePacketHeader(packet);
  m_dataStream->writeBytes(packet.constData(),
                           static_cast<unsigned int>(packet.size()));
  m_socket->flush();
}

void AbstractRpcInterface::replyToInvalidPacket(
    const Json::Value &packetId, const Json::Value &errorDataObject)
{
  DEBUGOUT("replyToInvalidPacket") "replying to an invalid packet.";
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32700, "Parse error", errorDataObject, packetId);
  this->sendPacket(packet);
}

void AbstractRpcInterface::replyToInvalidRequest(
    const Json::Value &packetId, const Json::Value &errorDataObject)
{
  DEBUGOUT("replyToInvalidRequest") "replying to an invalid request.";
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32600, "Invalid request", errorDataObject, packetId);
  this->sendPacket(packet);
}

void AbstractRpcInterface::replyToUnrecognizedRequest(
    const Json::Value &packetId, const Json::Value &errorDataObject)
{
  DEBUGOUT("replyToUnrecognizedRequest") "replying to an unrecognized method.";
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32601, "Method not found", errorDataObject, packetId);
  this->sendPacket(packet);
}

void AbstractRpcInterface::replyToinvalidRequestParams(
    const Json::Value &packetId, const Json::Value &errorDataObject)
{
  DEBUGOUT("replyToInvalidRequestParam") "replying to an ill-formed request.";
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32602, "Invalid params", errorDataObject, packetId);
  this->sendPacket(packet);
}

void AbstractRpcInterface::replyWithInternalError(
    const Json::Value &packetId, const Json::Value &errorDataObject)
{
  DEBUGOUT("replyWithInternalError") "Notifying peer of internal error.";
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32603, "Internal error", errorDataObject, packetId);
  this->sendPacket(packet);
}

IdType AbstractRpcInterface::nextPacketId()
{
  return m_packetCounter++;
}

void AbstractRpcInterface::writePacketHeader(const PacketType &packet)
{
  DEBUGOUT("writePacketHeader") "Writing packet header."
      << "Version:" << m_headerVersion
      << "Size:" << packet.size();

  // First write a version identifier
  (*m_dataStream) << m_headerVersion;

  // Next is the packet size as 32-bit unsigned integer
  (*m_dataStream) << static_cast<quint32>(packet.size());
}

bool AbstractRpcInterface::canReadPacketHeader()
{
  return (m_socket->bytesAvailable() >= m_headerSize);
}

quint32 AbstractRpcInterface::readPacketHeader()
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

  DEBUGOUT("readPacketHeader") "Reading packet header."
      << "Version:" << headerVersion
      << "Size:" << packetSize;

  return packetSize;
}

} // end namespace MoleQueue
