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
  qRegisterMetaType<JobState>("MoleQueue::JobState");
  qRegisterMetaType<QueueListType>("MoleQueue::QueueListType");
  qRegisterMetaType<JobSubmissionErrorCode>("MoleQueue::JobSubmissionErrorCode");
}

JsonRpc::~JsonRpc()
{
}

PacketType JsonRpc::generateErrorResponse(int errorCode,
                                          const QString &message,
                                          IdType packetId)
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
                                          IdType packetId)
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

void JsonRpc::interpretIncomingPacket(Connection* connection,
                                      const Message msg)
{
  // Read packet into a Json value
  Json::Reader reader;
  Json::Value root;

  if (!reader.parse(msg.data().constData(),
                    msg.data().constData() + msg.data().size(),
                    root, false)) {
    handleUnparsablePacket(connection, msg);
    return;
  }

  // Submit the root node for processing
  interpretIncomingJsonRpc(connection, msg.replyTo(), root);
}

void JsonRpc::interpretIncomingJsonRpc(Connection *connection,
                                       EndpointId replyTo,
                                       const Json::Value &data)
{
  // Handle batch requests recursively:
  if (data.isArray()) {
    for (Json::Value::const_iterator it = data.begin(), it_end = data.end();
         it != it_end; ++it) {
      interpretIncomingJsonRpc(connection, replyTo, *it);
    }

    return;
  }

  if (!data.isObject()) {
    handleInvalidRequest(connection, replyTo, data);
    return;
  }

  PacketForm form = guessPacketForm(data);
  int method = guessPacketMethod(data);

  // Validate detected type
  switch (form) {
  case REQUEST_PACKET:
    if (!validateRequest(data, false))
      form = INVALID_PACKET;
    break;
  case RESULT_PACKET:
  case ERROR_PACKET:
    if (!validateResponse(data, false))
      form = INVALID_PACKET;
    break;
  case NOTIFICATION_PACKET:
    if (!validateNotification(data, false))
      form = INVALID_PACKET;
    break;
  default:
  case INVALID_PACKET:
    break;
  }

  switch (method) {
  case IGNORE_METHOD:
    break;
  case INVALID_METHOD:
    handleInvalidRequest(connection, replyTo, data);
    break;
  case UNRECOGNIZED_METHOD:
    handleUnrecognizedRequest(connection, replyTo, data);
    break;
  default:
    handlePacket(method, form, connection, replyTo, data);
    break;
  }

  // Remove responses from pendingRequests lookup table
  // id is guaranteed to exist after earlier validation
  if (form == RESULT_PACKET || form == ERROR_PACKET)
    registerReply(static_cast<IdType>(data["id"].asLargestUInt()));
}

bool JsonRpc::validateRequest(const PacketType &packet, bool strict)
{
  Json::Value root;
  Json::Reader reader;
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  return validateRequest(root, strict);
}

bool JsonRpc::validateRequest(const Json::Value &packet, bool strict)
{
  if (!packet.isObject())
    return false;

  Json::Value::Members members = packet.getMemberNames();

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
  if (!packet["method"].isString())
    return false;

  // "params" may be omitted, but must be structured if present
  if (found_params &&
      !packet["params"].isObject() && !packet["params"].isArray()) {
      return false;
  }

  // "id" must be a string, a number, or null, but should not be null or
  // fractional
  const Json::Value & idValue = packet["id"];
  if (!idValue.isString() && !idValue.isNumeric() && !idValue.isNull())
    return false;

  // Print extra members
  if (extraMembers.size() != 0 && strict)
    return false;

  return true;
}

bool JsonRpc::validateResponse(const PacketType &packet, bool strict)
{
  Json::Value root;
  Json::Reader reader;
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  return validateResponse(root, strict);
}

bool JsonRpc::validateResponse(const Json::Value &packet, bool strict)
{
  if (!packet.isObject())
    return false;

  Json::Value::Members members = packet.getMemberNames();

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
    const Json::Value & errorObject = packet["error"];
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
  const Json::Value & idValue = packet["id"];
  if (!idValue.isString() && !idValue.isNumeric() && !idValue.isNull())
    return false;

  // Print extra members
  if (extraMembers.size() != 0 && strict)
    return false;

  return true;
}

bool JsonRpc::validateNotification(const PacketType &packet, bool strict)
{
  Json::Value root;
  Json::Reader reader;
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  return validateNotification(root, strict);
}

bool JsonRpc::validateNotification(const Json::Value &packet, bool strict)
{
  if (!packet.isObject())
    return false;

  Json::Value::Members members = packet.getMemberNames();

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
  if (!packet["method"].isString())
    return false;

  // "params" may be omitted, but must be structured if present
  if (found_params &&
      !packet["params"].isObject() && !packet["params"].isArray()) {
    return false;
  }

  // Print extra members
  if (extraMembers.size() != 0 && strict)
    return false;

  return true;
}

Json::Value JsonRpc::generateEmptyRequest(IdType id)
{
  Json::Value ret (Json::objectValue);
  ret["jsonrpc"] = "2.0";
  ret["method"] = Json::nullValue;
  ret["id"] = id;

  return ret;
}

Json::Value JsonRpc::generateEmptyResponse(IdType id)
{
  Json::Value ret (Json::objectValue);
  ret["jsonrpc"] = "2.0";
  ret["result"] = Json::nullValue;
  ret["id"] = id;

  return ret;
}

Json::Value JsonRpc::generateEmptyError(IdType id)
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

JsonRpc::PacketForm JsonRpc::guessPacketForm(const Json::Value &root) const
{
  if (!root.isObject())
    return INVALID_PACKET;

  if (root["method"] != Json::nullValue) {
    if (root["id"] != Json::nullValue)
      return REQUEST_PACKET;
    else
      return NOTIFICATION_PACKET;
  }
  else if (root["result"] != Json::nullValue)
    return RESULT_PACKET;
  else if (root["error"] != Json::nullValue)
    return ERROR_PACKET;

  return INVALID_PACKET;
}

int JsonRpc::guessPacketMethod(const Json::Value &root) const
{
  if (!root.isObject())
    return INVALID_METHOD;

  const Json::Value & methodValue = root["method"];

  if (methodValue != Json::nullValue) {
    if (!methodValue.isString())
      return INVALID_METHOD;

    const char *methodCString = methodValue.asCString();

    return mapMethodNameToInt(QString(methodCString));
  }

  // No method present -- this is a reply. Determine if it's a reply to this
  // client.
  const Json::Value & idValue = root["id"];

  if (idValue != Json::nullValue) {
    // We will only submit unsigned integral ids
    if (!idValue.isIntegral())
      return IGNORE_METHOD;

    IdType packetId = static_cast<IdType>(idValue.asLargestUInt());

    if (m_pendingRequests.contains(packetId)) {
      return m_pendingRequests[packetId];
    }

    // if the packetId isn't in the lookup table, it's not from us
    return IGNORE_METHOD;
  }

  // No method or id present?
  return INVALID_METHOD;
}

void JsonRpc::handleUnparsablePacket(Connection *connection,
                                     const Message msg) const
{
  Json::Value errorData (Json::objectValue);

  errorData["receivedPacket"] = msg.data().constData();

  emit invalidPacketReceived(connection, msg.replyTo(), Json::nullValue,
                             errorData);
}

void JsonRpc::handleInvalidRequest(Connection *connection,
                                   EndpointId replyTo,
                                   const Json::Value &root) const
{
  Json::Value errorData (Json::objectValue);

  errorData["receivedJson"] = Json::Value(root);

  emit invalidRequestReceived(connection, replyTo,
                              (root.isObject()) ? root["id"] : Json::nullValue,
                              errorData);
}

void JsonRpc::handleUnrecognizedRequest(Connection *connection,
                                        EndpointId replyTo,
                                        const Json::Value &root) const
{
  Json::Value errorData (Json::objectValue);

  errorData["receivedJson"] = root;

  emit unrecognizedRequestReceived(connection, replyTo, root["id"], errorData);
}

void JsonRpc::registerRequest(IdType packetId, int method)
{
  m_pendingRequests[packetId] = method;
}

void JsonRpc::registerReply(IdType packetId)
{
  m_pendingRequests.remove(packetId);
}

} // end namespace MoleQueue
