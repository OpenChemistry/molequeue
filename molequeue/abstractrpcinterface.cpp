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
#include "connection.h"

#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtGlobal>

#define DEBUGOUT(title) \
  if (this->m_debug)    \
    qDebug() << QDateTime::currentDateTime().toString() \
             << "AbstractRpcInterface" << title <<

namespace MoleQueue
{

AbstractRpcInterface::AbstractRpcInterface(QObject *parentObject) :
  QObject(parentObject),
  m_connection(NULL),
  m_jsonrpc(new JsonRpc (this)),
  m_packetCounter(0),
  m_debug(false)
{
  // Randomize the packet counter's starting value.
  qsrand(QDateTime::currentMSecsSinceEpoch());
  m_packetCounter = static_cast<IdType>(qrand());

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
}

AbstractRpcInterface::~AbstractRpcInterface()
{
  if (m_connection) {
    m_connection->close();
    delete m_connection;
    m_connection = NULL;
  }

  delete m_jsonrpc;
  m_jsonrpc = NULL;
}

void AbstractRpcInterface::setConnection(Connection *connection)
{
  m_connection = connection;
  connect(connection, SIGNAL(newMessage(const MoleQueue::PacketType&)),
          this, SLOT(readPacket(const MoleQueue::PacketType&)));
}

void AbstractRpcInterface::readPacket(const PacketType &packet)
{
  DEBUGOUT("readPacket") "Interpreting new packet.";
  m_jsonrpc->interpretIncomingPacket(packet);
}

void AbstractRpcInterface::replyToInvalidPacket(
    const Json::Value &packetId, const Json::Value &errorDataObject)
{
  DEBUGOUT("replyToInvalidPacket") "replying to an invalid packet.";
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32700, "Parse error", errorDataObject, packetId);
  m_connection->send(packet);
}

void AbstractRpcInterface::replyToInvalidRequest(
    const Json::Value &packetId, const Json::Value &errorDataObject)
{
  DEBUGOUT("replyToInvalidRequest") "replying to an invalid request.";
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32600, "Invalid request", errorDataObject, packetId);
  m_connection->send(packet);
}

void AbstractRpcInterface::replyToUnrecognizedRequest(
    const Json::Value &packetId, const Json::Value &errorDataObject)
{
  DEBUGOUT("replyToUnrecognizedRequest") "replying to an unrecognized method.";
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32601, "Method not found", errorDataObject, packetId);
  m_connection->send(packet);
}

void AbstractRpcInterface::replyToinvalidRequestParams(
    const Json::Value &packetId, const Json::Value &errorDataObject)
{
  DEBUGOUT("replyToInvalidRequestParam") "replying to an ill-formed request.";
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32602, "Invalid params", errorDataObject, packetId);
  m_connection->send(packet);
}

void AbstractRpcInterface::replyWithInternalError(
    const Json::Value &packetId, const Json::Value &errorDataObject)
{
  DEBUGOUT("replyWithInternalError") "Notifying peer of internal error.";
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32603, "Internal error", errorDataObject, packetId);
  m_connection->send(packet);
}

IdType AbstractRpcInterface::nextPacketId()
{
  return m_packetCounter++;
}

} // end namespace MoleQueue
