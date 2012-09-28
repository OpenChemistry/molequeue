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
#include "message.h"

#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtGlobal>
#include "assert.h"

namespace MoleQueue
{

AbstractRpcInterface::AbstractRpcInterface(QObject *parentObject) :
  QObject(parentObject),
  m_jsonrpc(NULL),
  m_messageIdGenerator(0)
{
  // Randomize the packet counter's starting value.
  qsrand(static_cast<uint>(QDateTime::currentMSecsSinceEpoch()));
  m_messageIdGenerator = static_cast<IdType>(qrand());
}

AbstractRpcInterface::~AbstractRpcInterface()
{
  delete m_jsonrpc;
  m_jsonrpc = NULL;
}

void AbstractRpcInterface::readMessage(const MoleQueue::Message msg)
{
  m_jsonrpc->interpretIncomingMessage(msg);
}

void AbstractRpcInterface::replyToInvalidMessage(
    const Message &request, const Json::Value &errorDataObject)
{
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32700, "Parse error", errorDataObject, request.id());
  Message msg(request, packet);
  msg.send();
}

void AbstractRpcInterface::replyToInvalidRequest(
    const Message &request, const Json::Value &errorDataObject)
{
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32600, "Invalid request", errorDataObject, request.id());
  Message msg(request, packet);
  msg.send();
}

void AbstractRpcInterface::replyToUnrecognizedRequest(
    const Message &request, const Json::Value &errorDataObject)
{
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32601, "Method not found", errorDataObject, request.id());
  Message msg(request, packet);
  msg.send();
}

void AbstractRpcInterface::replyToinvalidRequestParams(
    const Message &request, const Json::Value &errorDataObject)
{
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32602, "Invalid params", errorDataObject, request.id());
  Message msg(request, packet);
  msg.send();
}

void AbstractRpcInterface::replyWithInternalError(
    const Message &request, const Json::Value &errorDataObject)
{
  PacketType packet = m_jsonrpc->generateErrorResponse(
        -32603, "Internal error", errorDataObject, request.id());
  Message msg(request, packet);
  msg.send();
}

void AbstractRpcInterface::setJsonRpc(JsonRpc *jsonrpc)
{
  m_jsonrpc = jsonrpc;

  connect(m_jsonrpc, SIGNAL(invalidMessageReceived(MoleQueue::Message,
                                                   Json::Value)),
          this, SLOT(replyToInvalidMessage(MoleQueue::Message, Json::Value)));

  connect(m_jsonrpc, SIGNAL(invalidRequestReceived(MoleQueue::Message,
                                                   Json::Value)),
          this, SLOT(replyToInvalidRequest(MoleQueue::Message, Json::Value)));

  connect(m_jsonrpc, SIGNAL(unrecognizedRequestReceived(MoleQueue::Message,
                                                        Json::Value)),
          this, SLOT(replyToUnrecognizedRequest(MoleQueue::Message,
                                                Json::Value)));

  connect(m_jsonrpc, SIGNAL(invalidRequestParamsReceived(MoleQueue::Message,
                                                         Json::Value)),
          this, SLOT(replyToinvalidRequestParams(MoleQueue::Message,
                                                 Json::Value)));

  connect(m_jsonrpc, SIGNAL(internalErrorOccurred(MoleQueue::Message,
                                                  Json::Value)),
          this, SLOT(replyWithInternalError(MoleQueue::Message, Json::Value)));
}

MessageIdType AbstractRpcInterface::nextMessageId()
{
  // Parsed integral ids will have int type, not uint type. Casting to int here
  // keeps the lookup tables working.
  return MessageIdType(static_cast<int>(m_messageIdGenerator++));
}

} // end namespace MoleQueue
