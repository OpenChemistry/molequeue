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

#include "jsonrpc.h"

#include "molequeueglobal.h"
#include "qtjson.h"

#include <json/json.h>

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QVariantHash>

namespace MoleQueue
{

JsonRpc::JsonRpc(QObject *parentObject)
  : QObject(parentObject)
{
  qRegisterMetaType<QDir>("QDir");
  qRegisterMetaType<Json::Value>("Json::Value");
  qRegisterMetaType<IdType>("MoleQueue::IdType");
  qRegisterMetaType<Message>("MoleQueue::Message");
  qRegisterMetaType<MessageIdType>("MoleQueue::MessageIdType");
  qRegisterMetaType<JobState>("MoleQueue::JobState");
  qRegisterMetaType<QueueListType>("MoleQueue::QueueListType");
  qRegisterMetaType<ErrorCode>("MoleQueue::ErrorCode");
}

JsonRpc::~JsonRpc()
{
}

PacketType JsonRpc::generateErrorResponse(int errorCode,
                                          const QString &message,
                                          const MessageIdType &packetId)
{
  Json::Value packet = generateEmptyError(packetId);

  packet["error"]["code"]    = errorCode;
  packet["error"]["message"] = message.toStdString();

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret (ret_stdstr.c_str());

  return ret;
}

PacketType JsonRpc::generateErrorResponse(int errorCode,
                                          const QString &message,
                                          const Json::Value &data,
                                          const MessageIdType &packetId)
{
  Json::Value packet = generateEmptyError(packetId);

  packet["error"]["code"]    = errorCode;
  packet["error"]["message"] = message.toStdString();
  packet["error"]["data"]    = data;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret (ret_stdstr.c_str());

  return ret;
}

PacketType JsonRpc::generateErrorResponse(int errorCode,
                                          const QString &message,
                                          const Json::Value &packetId)
{
  Json::Value packet = generateEmptyError(packetId);

  packet["error"]["code"]    = errorCode;
  packet["error"]["message"] = message.toStdString();

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret (ret_stdstr.c_str());

  return ret;
}

PacketType JsonRpc::generateErrorResponse(int errorCode,
                                          const QString &message,
                                          const Json::Value &data,
                                          const Json::Value &packetId)
{
  Json::Value packet = generateEmptyError(packetId);

  packet["error"]["code"]    = errorCode;
  packet["error"]["message"] = message.toStdString();
  packet["error"]["data"]    = data;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret (ret_stdstr.c_str());

  return ret;
}

void JsonRpc::interpretIncomingMessage(const Message &msg_orig)
{
  Message msg(msg_orig);
  // Parse the message contents if the json value is null.
  if (msg.json().isNull()) {
    // A truly empty message? Nothing to do!
    if (msg.data().isEmpty())
      return;
    if (!msg.parse()) {
      handleUnparsableMessage(msg);
      return;
    }
  }

  // Handle batch requests recursively:
  if (msg.json().isArray()) {
    for (Json::Value::const_iterator it = msg.json().begin(),
         it_end = msg.json().end(); it != it_end; ++it) {
      Message subMessage(msg);
      subMessage.setJson(*it);
      interpretIncomingMessage(subMessage);
    }
    return;
  }
  if (!msg.json().isObject()) {
    handleInvalidRequest(msg);
    return;
  }

  msg.setType(guessMessageType(msg));

  // Validate detected type
  switch (msg.type()) {
  case Message::REQUEST_MESSAGE:
    if (!validateRequest(msg, false))
      msg.setType(Message::INVALID_MESSAGE);
    break;
  case Message::RESULT_MESSAGE:
  case Message::ERROR_MESSAGE:
    if (!validateResponse(msg, false))
      msg.setType(Message::INVALID_MESSAGE);
    break;
  case Message::NOTIFICATION_MESSAGE:
    if (!validateNotification(msg, false))
      msg.setType(Message::INVALID_MESSAGE);
    break;
  default:
  case Message::INVALID_MESSAGE:
    break;
  }

  // Set id if needed -- must be done before guessMessageMethod()
  switch (msg.type()) {
  case Message::REQUEST_MESSAGE:
  case Message::RESULT_MESSAGE:
  case Message::ERROR_MESSAGE: {
    const Json::Value &idmember = msg.json()["id"];
    if (idmember.isNull()) {
      msg.setIdToNull();
    }
    else {
      if (idmember.isNull()) {
        msg.setIdToNull();
      }
      else if (idmember.isString()) {
        msg.setId(MessageIdType(idmember.asCString()));
      }
      else {
        // This isn't strictly valid JSON-RPC...just convert it to a string.
        msg.setId(MessageIdType(idmember.toStyledString().c_str()));
      }
    }
  }
    break;
  default:
  case Message::INVALID_MESSAGE:
  case Message::NOTIFICATION_MESSAGE:
    break;
  }

  int method = guessMessageMethod(msg);

  switch (method) {
  case IGNORE_METHOD:
    break;
  case INVALID_METHOD:
    handleInvalidRequest(msg);
    break;
  case UNRECOGNIZED_METHOD:
    handleUnrecognizedRequest(msg);
    break;
  default:
    handleMessage(method, msg);
    break;
  }

  // Remove responses from pendingRequests lookup table
  // id is guaranteed to exist after earlier validation
  if (msg.type() == Message::RESULT_MESSAGE ||
      msg.type() == Message::ERROR_MESSAGE) {
    registerReply(msg.id());
  }
}

bool JsonRpc::validateRequest(const Message &msg, bool strict)
{
  if (!msg.json().isObject())
    return false;

  Json::Value::Members members = msg.json().getMemberNames();

  // Check that the required members are present
  bool found_jsonrpc = false;
  bool found_method = false;
  bool found_params = false;
  bool found_id = false;
  Json::Value::Members extraMembers;

  for (Json::Value::Members::const_iterator it = members.begin(),
       it_end = members.end(); it != it_end; ++it) {
    if (!found_jsonrpc && it->compare("jsonrpc") == 0) {
      found_jsonrpc = true;
    }
    else if (!found_method && it->compare("method") == 0) {
      found_method = true;
    }
    else if (!found_params && it->compare("params") == 0) {
      found_params = true;
    }
    else if (!found_id && it->compare("id") == 0) {
      found_id = true;
    }
    else {
      extraMembers.push_back(*it);
    }
  }

  if (!found_jsonrpc && strict)
      return false;

  if (!found_method)
    return false;

  // Params are optional.
  //  if (!found_params)
  //    return false;

  if (!found_id)
    return false;

  // Validate objects
  // "method" must be a string
  if (!msg.json()["method"].isString())
    return false;

  // "params" may be omitted, but must be structured if present
  if (found_params &&
      !msg.json()["params"].isObject() && !msg.json()["params"].isArray()) {
      return false;
  }

  // "id" must be a string, a number, or null, but should not be null or
  // fractional
  const Json::Value & idValue = msg.json()["id"];
  if (strict) {
    if (!idValue.isString())
      return false;
  }
  else if (!idValue.isString() && !idValue.isNumeric() && !idValue.isNull()) {
    return false;
  }

  // Print extra members
  if (extraMembers.size() != 0 && strict)
    return false;

  return true;
}

bool JsonRpc::validateResponse(const Message &msg, bool strict)
{
  if (!msg.json().isObject())
    return false;

  Json::Value::Members members = msg.json().getMemberNames();

  // Check that the required members are present
  bool found_jsonrpc = false;
  bool found_result = false;
  bool found_error = false;
  bool found_id = false;
  Json::Value::Members extraMembers;

  for (Json::Value::Members::const_iterator it = members.begin(),
       it_end = members.end(); it != it_end; ++it) {
    if (!found_jsonrpc && it->compare("jsonrpc") == 0) {
      found_jsonrpc = true;
    }
    else if (!found_result && it->compare("result") == 0) {
      found_result = true;
    }
    else if (!found_error && it->compare("error") == 0) {
      found_error = true;
    }
    else if (!found_id && it->compare("id") == 0) {
      found_id = true;
    }
    else {
      extraMembers.push_back(*it);
    }
  }

  if (!found_jsonrpc && strict)
    return false;

  if (!found_result && !found_error)
    return false;

  if (found_result && found_error)
    return false;

  if (!found_id)
    return false;

  // Validate error object if present
  if (found_error) {
    const Json::Value & errorObject = msg.json()["error"];
    if (!errorObject.isObject())
      return false;

    // "code" must be an integer
    if (!errorObject["code"].isIntegral())
      return false;

    // "message" must be a string
    if (!errorObject["message"].isString())
      return false;
  }

  // "id" must be a string, a number, or null, but should not be null or
  // fractional
  const Json::Value & idValue = msg.json()["id"];
  if (strict) {
    if (!idValue.isString())
      return false;
  }
  else if (!idValue.isString() && !idValue.isNumeric() && !idValue.isNull()) {
    return false;
  }

  // Print extra members
  if (extraMembers.size() != 0 && strict)
    return false;

  return true;
}

bool JsonRpc::validateNotification(const Message &msg, bool strict)
{
  if (!msg.json().isObject())
    return false;

  Json::Value::Members members = msg.json().getMemberNames();

  // Check that the required members are present
  bool found_jsonrpc = false;
  bool found_method = false;
  bool found_params = false;
  bool found_id = false;
  Json::Value::Members extraMembers;

  for (Json::Value::Members::const_iterator it = members.begin(),
       it_end = members.end(); it != it_end; ++it) {
    if (!found_jsonrpc && it->compare("jsonrpc") == 0) {
      found_jsonrpc = true;
    }
    else if (!found_method && it->compare("method") == 0) {
      found_method = true;
    }
    else if (!found_params && it->compare("params") == 0) {
      found_params = true;
    }
    else if (!found_id && it->compare("id") == 0) {
      found_id = true;
    }
    else {
      extraMembers.push_back(*it);
    }
  }

  if (!found_jsonrpc && strict)
      return false;

  if (!found_method)
    return false;

  // Params are optional.
  //  if (!found_params)
  //    return false;

  if (found_id)
    return false;

  // Validate objects
  // "method" must be a string
  if (!msg.json()["method"].isString())
    return false;

  // "params" may be omitted, but must be structured if present
  if (found_params &&
      !msg.json()["params"].isObject() && !msg.json()["params"].isArray()) {
    return false;
  }

  // Print extra members
  if (extraMembers.size() != 0 && strict)
    return false;

  return true;
}

Json::Value JsonRpc::generateEmptyRequest(const MessageIdType &id)
{
  Json::Value ret (Json::objectValue);
  ret["jsonrpc"] = "2.0";
  ret["method"] = Json::nullValue;
  ret["id"] = id.isNull() ? Json::Value(Json::nullValue)
                          : Json::Value(id.data());

  return ret;
}

Json::Value JsonRpc::generateEmptyResponse(const MessageIdType &id)
{
  Json::Value ret (Json::objectValue);
  ret["jsonrpc"] = "2.0";
  ret["result"] = Json::nullValue;
  ret["id"] = id.isNull() ? Json::Value(Json::nullValue)
                          : Json::Value(id.data());

  return ret;
}

Json::Value JsonRpc::generateEmptyError(const MessageIdType &id)
{
  Json::Value ret (Json::objectValue);
  ret["jsonrpc"] = "2.0";
  Json::Value errorValue (Json::objectValue);
  errorValue["code"] = Json::nullValue;
  errorValue["message"] = Json::nullValue;
  ret["error"] = Json::nullValue;
  ret["id"] = id.isNull() ? Json::Value(Json::nullValue)
                          : Json::Value(id.data());

  return ret;
}

Json::Value JsonRpc::generateEmptyError(const Json::Value &id)
{
  Json::Value ret (Json::objectValue);
  ret["jsonrpc"] = "2.0";
  Json::Value errorValue (Json::objectValue);
  errorValue["code"] = Json::nullValue;
  errorValue["message"] = Json::nullValue;
  ret["error"] = Json::nullValue;
  ret["id"] = id;

  return ret;
}

Json::Value JsonRpc::generateEmptyNotification()
{
  Json::Value ret (Json::objectValue);
  ret["jsonrpc"] = "2.0";
  ret["method"] = Json::nullValue;

  return ret;
}

Message::Type JsonRpc::guessMessageType(const Message &msg) const
{
  if (!msg.json().isObject())
    return Message::INVALID_MESSAGE;

  if (msg.json()["method"] != Json::nullValue) {
    if (msg.json()["id"] != Json::nullValue)
      return Message::REQUEST_MESSAGE;
    else
      return Message::NOTIFICATION_MESSAGE;
  }
  else if (msg.json()["result"] != Json::nullValue)
    return Message::RESULT_MESSAGE;
  else if (msg.json()["error"] != Json::nullValue)
    return Message::ERROR_MESSAGE;

  return Message::INVALID_MESSAGE;
}

int JsonRpc::guessMessageMethod(const Message &msg) const
{
  if (!msg.json().isObject())
    return INVALID_METHOD;

  const Json::Value & methodValue = msg.json()["method"];

  if (methodValue != Json::nullValue) {
    if (!methodValue.isString())
      return INVALID_METHOD;

    return mapMethodNameToInt(QString(methodValue.asCString()));
  }

  // No method present -- this is a reply. Determine if it's a reply to this
  // client.
  if (!msg.idIsNull()) {
    return m_pendingRequests.value(msg.id(), IGNORE_METHOD);
  }

  // No method or id present?
  return INVALID_METHOD;
}

void JsonRpc::handleUnparsableMessage(const Message &msg) const
{
  Json::Value errorData(Json::objectValue);
  errorData["receivedPacket"] = msg.data().constData();
  emit invalidMessageReceived(msg, errorData);
}

void JsonRpc::handleInvalidRequest(const Message &msg) const
{
  Json::Value errorData(Json::objectValue);
  errorData["receivedJson"] = msg.json();
  emit invalidRequestReceived(msg, errorData);
}

void JsonRpc::handleUnrecognizedRequest(const Message &msg) const
{
  Json::Value errorData(Json::objectValue);
  errorData["receivedJson"] = msg.json();
  emit unrecognizedRequestReceived(msg, errorData);
}

void JsonRpc::registerRequest(const MessageIdType &packetId, int method)
{
  m_pendingRequests[packetId] = method;
}

void JsonRpc::registerReply(const MessageIdType &packetId)
{
  m_pendingRequests.remove(packetId);
}

} // end namespace MoleQueue
