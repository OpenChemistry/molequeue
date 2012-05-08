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

#include "jobrequest.h"
#include "molequeueglobal.h"
#include "program.h"
#include "queue.h"
#include "queuemanager.h"

#include "thirdparty/jsoncpp/json/json.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QVariantHash>

#define DEBUGOUT(title) \
  if (this->m_debug)    \
    qDebug() << QDateTime::currentDateTime().toString() << title <<

namespace MoleQueue
{

JsonRpc::JsonRpc(QObject *parentObject)
  : QObject(parentObject),
    m_debug(false)
{
}

JsonRpc::~JsonRpc()
{
  if (m_debug && m_pendingRequests.size() != 0) {
    DEBUGOUT("~JsonRpc") "Dangling requests upon destruction:";
    for (QHash<mqIdType, PacketMethod>::const_iterator
         it     = m_pendingRequests.constBegin(),
         it_end = m_pendingRequests.constEnd(); it != it_end; ++it) {
      DEBUGOUT("!JsonRpc") "    PacketId:" << it.key() << "Request Method:"
                                           << it.value();
    }
  }
}

mqPacketType JsonRpc::generateJobRequest(const JobRequest &req,
                                         mqIdType packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "submitJob";

  Json::Value paramsObject (Json::objectValue);
  paramsObject["queue"]               = req.queue().toStdString();
  paramsObject["program"]             = req.program().toStdString();
  paramsObject["description"]         = req.description().toStdString();
  paramsObject["cleanRemoteFiles"]    = req.cleanRemoteFiles();
  paramsObject["retrieveOutput"]      = req.retrieveOutput();
  paramsObject["outputDirectory"]     = req.outputDirectory().toStdString();
  paramsObject["cleanLocalWorkingDirectory"] = req.cleanLocalWorkingDirectory();
  paramsObject["hideFromQueue"]       = req.hideFromQueue();
  paramsObject["popupOnStateChange"]  = req.popupOnStateChange();
  if (req.inputAsPath().isEmpty())
    paramsObject["inputAsString"]     = req.inputAsString().toStdString();
  else
    paramsObject["inputAsPath"]       = req.inputAsPath().toStdString();

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());


  DEBUGOUT("generateJobRequest") "New job request:\n" << ret;

  this->registerRequest(packetId, SUBMIT_JOB);

  return ret;
}

mqPacketType
JsonRpc::generateJobSubmissionConfirmation(mqIdType moleQueueJobId,
                                           mqIdType queueJobId,
                                           const QString &workingDirectory,
                                           mqIdType packetId)
{
  Json::Value packet = generateEmptyResponse(packetId);

  Json::Value resultObject (Json::objectValue);
  resultObject["moleQueueJobId"] = moleQueueJobId;
  resultObject["queueJobId"] = queueJobId;
  resultObject["workingDirectory"] = workingDirectory.toStdString();

  packet["result"] = resultObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobSubmissionConfirmation")
      "New job confirmation generated:\n" << ret;

  return ret;
}

mqPacketType JsonRpc::generateErrorResponse(int errorCode,
                                            const QString &message,
                                            mqIdType packetId)
{
  Json::Value packet = generateEmptyError(packetId);

  packet["error"]["code"]    = errorCode;
  packet["error"]["message"] = message.toStdString();

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateErrorResponse")
      "New error response generated:\n" << ret;

  return ret;
}

mqPacketType JsonRpc::generateErrorResponse(int errorCode,
                                            const QString &message,
                                            const Json::Value &data,
                                            mqIdType packetId)
{
  Json::Value packet = generateEmptyError(packetId);

  packet["error"]["code"]    = errorCode;
  packet["error"]["message"] = message.toStdString();
  packet["error"]["data"]    = data;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateErrorResponse")
      "New error response generated:\n" << ret;

  return ret;
}

mqPacketType JsonRpc::generateErrorResponse(int errorCode,
                                            const QString &message,
                                            const Json::Value &packetId)
{
  Json::Value packet = generateEmptyError(packetId);

  packet["error"]["code"]    = errorCode;
  packet["error"]["message"] = message.toStdString();

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateErrorResponse")
      "New error response generated:\n" << ret;

  return ret;
}

mqPacketType JsonRpc::generateErrorResponse(int errorCode,
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
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateErrorResponse")
      "New error response generated:\n" << ret;

  return ret;
}

mqPacketType JsonRpc::generateJobCancellation(const JobRequest &req,
                                              mqIdType packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "cancelJob";

  Json::Value paramsObject (Json::objectValue);
  paramsObject["moleQueueJobId"] = req.molequeueId();

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobCancellation") "New job cancellation request:\n" << ret;

  this->registerRequest(packetId, CANCEL_JOB);

  return ret;
}

mqPacketType JsonRpc::generateJobCancellationConfirmation(mqIdType moleQueueId,
                                                          mqIdType packetId)
{
  Json::Value packet = generateEmptyResponse(packetId);

  packet["result"] = moleQueueId;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobCancellationConfirmation")
      "New job cancellation confirmation generated:\n" << ret;

  return ret;
}

mqPacketType JsonRpc::generateQueueListRequest(mqIdType packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "listQueues";

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateQueueListRequest") "New queue list request:\n" << ret;

  this->registerRequest(packetId, LIST_QUEUES);

  return ret;
}

mqPacketType JsonRpc::generateQueueList(const QueueManager *qmanager,
                                        mqIdType packetId)
{
  if (!qmanager) {
    qDebug() << Q_FUNC_INFO << "called with a NULL QueueManager?";
    return mqPacketType();
  }
  Json::Value packet = generateEmptyResponse(packetId);

  Json::Value resultObject (Json::objectValue);
  foreach (const Queue *queue, qmanager->queues()) {
    const std::string queueName = queue->name().toStdString();

    Json::Value programArray (Json::arrayValue);
    foreach (const QString prog, queue->programs()) {
      const std::string progName = prog.toStdString();
      programArray.append(progName);
    }
    resultObject[queueName] = programArray;
  }

  packet["result"] = resultObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateQueueList") "Queue list generated:\n" << ret;

  return ret;
}

mqPacketType
JsonRpc::generateJobStateChangeNotification(mqIdType moleQueueJobId,
                                            JobState oldState,
                                            JobState newState)
{
  Json::Value packet = generateEmptyNotification();

  packet["method"] = "jobStateChanged";

  Json::Value paramsObject (Json::objectValue);
  paramsObject["moleQueueJobId"]      = moleQueueJobId;
  paramsObject["oldState"]            = jobStateToString(oldState);
  paramsObject["newState"]            = jobStateToString(newState);

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobStateChangeNotification")
      "New state change:\n" << ret;

  return ret;
}

void JsonRpc::interpretIncomingPacket(const mqPacketType &packet)
{
  // Read packet into a Json value
  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(packet.constData(), packet.constData() + packet.size(),
                    root, false)) {
    this->handleUnparsablePacket(packet);
  }

  // Submit the root node for processing
  this->interpretIncomingJsonRpc(root);
}

void JsonRpc::interpretIncomingJsonRpc(const Json::Value &data)
{
  // Handle batch requests recursively:
  if (data.isArray()) {
    for (Json::Value::const_iterator it = data.begin(), it_end = data.end();
         it != it_end; ++it) {
      this->interpretIncomingJsonRpc(*it);
    }

    return;
  }

  if (!data.isObject()) {
    this->handleInvalidRequest(data);
  }

  PacketType   type   = this->guessPacketType(data);
  PacketMethod method = this->guessPacketMethod(data);

  // Validate detected type
  switch (type) {
  case REQUEST_PACKET:
    if (!this->validateRequest(data, false))
      type = INVALID_PACKET;
    break;
  case RESULT_PACKET:
  case ERROR_PACKET:
    if (!this->validateResponse(data, false))
      type = INVALID_PACKET;
    break;
  case NOTIFICATION_PACKET:
    if (!this->validateNotification(data, false))
      type = INVALID_PACKET;
    break;
  default:
  case INVALID_PACKET:
    break;
  }

  switch (method) {
  case IGNORE_METHOD:
    DEBUGOUT("interpretIncomingJsonRpc") "Ignoring reply to other client.";
    break;
  default:
  case INVALID_METHOD:
    this->handleInvalidRequest(data);
    break;
  case UNRECOGNIZED_METHOD:
    this->handleUnrecognizedRequest(data);
    break;
  case LIST_QUEUES:
  {
    switch (type) {
    default:
    case INVALID_PACKET:
    case NOTIFICATION_PACKET:
      this->handleInvalidRequest(data);
      break;
    case REQUEST_PACKET:
      this->handleListQueuesRequest(data);
      break;
    case RESULT_PACKET:
      this->handleListQueuesResult(data);
      break;
    case ERROR_PACKET:
      this->handleListQueuesError(data);
      break;
    }
    break;
  }
  case SUBMIT_JOB:
  {
    switch (type) {
    default:
    case INVALID_PACKET:
    case NOTIFICATION_PACKET:
      this->handleInvalidRequest(data);
      break;
    case REQUEST_PACKET:
      this->handleSubmitJobRequest(data);
      break;
    case RESULT_PACKET:
      this->handleSubmitJobResult(data);
      break;
    case ERROR_PACKET:
      this->handleSubmitJobError(data);
      break;
    }
    break;
  }
  case CANCEL_JOB:
  {
    switch (type) {
    default:
    case INVALID_PACKET:
    case NOTIFICATION_PACKET:
      this->handleInvalidRequest(data);
      break;
    case REQUEST_PACKET:
      this->handleCancelJobRequest(data);
      break;
    case RESULT_PACKET:
      this->handleCancelJobResult(data);
      break;
    case ERROR_PACKET:
      this->handleCancelJobError(data);
      break;
    }
    break;
  }
  case JOB_STATE_CHANGED:
  {
    switch (type) {
    default:
    case INVALID_PACKET:
    case REQUEST_PACKET:
    case RESULT_PACKET:
    case ERROR_PACKET:
      this->handleInvalidRequest(data);
      break;
    case NOTIFICATION_PACKET:
      this->handleJobStateChangedNotification(data);
      break;
    }
    break;
  }
  }

  // Remove responses from pendingRequests lookup table
  // id is guaranteed to exist after earlier validation
  if (type == RESULT_PACKET || type == ERROR_PACKET)
    registerReply(static_cast<mqIdType>(data["id"].asLargestUInt()));

}

bool JsonRpc::validateRequest(const mqPacketType &packet, bool strict)
{
  Json::Value root;
  Json::Reader reader;
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  return this->validateRequest(root, strict);
}

bool JsonRpc::validateRequest(const Json::Value &packet, bool strict)
{
  if (!packet.isObject()) {
    DEBUGOUT("validateRequest") "Invalid: Root node is not an object!";
    return false;
  }

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

  if (!found_jsonrpc) {
    DEBUGOUT("validateRequest") "Warn: 'jsonrpc' not found!";
    if (strict)
      return false;
  }
  if (!found_method) {
    DEBUGOUT("validateRequest") "Invalid: 'method' not found!";
    return false;
  }
// Params are optional.
//  if (!found_params) {
//    DEBUGOUT("validateRequest") "Invalid: 'params' not found!";
//    return false;
//  }
  if (!found_id) {
    DEBUGOUT("validateRequest") "Invalid: 'id' not found!";
    return false;
  }

  // Validate objects
  // "method" must be a string
  if (!packet["method"].isString()) {
    DEBUGOUT("validateRequest") "Invalid: 'method' is not a string!";
    return false;
  }
  // "params" may be omitted, but must be structured if present
  if (found_params) {
    if (!packet["params"].isObject() && !packet["params"].isArray()) {
      DEBUGOUT("validateRequest") "Invalid: 'params' must be either an array "
          "or an object!";
      return false;
    }
  }
  // "id" must be a string, a number, or null, but should not be null or
  // fractional
  const Json::Value & idValue = packet["id"];
  if (!idValue.isString() && !idValue.isNumeric() && !idValue.isNull()) {
    DEBUGOUT("validateRequest") "Invalid: id value must be a string, a number, "
        "or null.";
    return false;
  }
  else if (idValue.isNumeric() && !idValue.isIntegral()) {
    DEBUGOUT("validateRequest") "Caution: 'id' should be integral if numeric.";
  }
  else if (idValue.isNull()) {
    DEBUGOUT("validateRequest") "Caution: 'id' should not be null.";
  }

  // Print extra members
  if (extraMembers.size() != 0) {
    if (this->m_debug) {
      std::string extra;
      for (Json::Value::Members::const_iterator it = extraMembers.begin(),
           it_end = extraMembers.end(); it != it_end; ++it) {
        if (it != extraMembers.begin())
          extra += ", ";
        extra += *it;
      }
      DEBUGOUT("validateRequest") "Warn: Extra top-level members found:"
          << extra.c_str();
    }

    if (strict)
      return false;
  }

  return true;
}

bool JsonRpc::validateResponse(const mqPacketType &packet, bool strict)
{
  Json::Value root;
  Json::Reader reader;
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  return this->validateResponse(root, strict);
}

bool JsonRpc::validateResponse(const Json::Value &packet, bool strict)
{
  if (!packet.isObject()) {
    DEBUGOUT("validateResponse") "Invalid: Root node is not an object!";
    return false;
  }

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

  if (!found_jsonrpc) {
    DEBUGOUT("validateResponse") "Warn: 'jsonrpc' not found!";
    if (strict)
      return false;
  }
  if (!found_result && !found_error) {
    DEBUGOUT("validateResponse") "Invalid: neither 'result' nor 'error' found!";
    return false;
  }
  if (found_result && found_error) {
    DEBUGOUT("validateResponse") "Invalid: both 'result' and 'error' present!";
    return false;
  }
  if (!found_id) {
    DEBUGOUT("validateResponse") "Invalid: 'id' not found!";
    return false;
  }

  // Validate error object if present
  if (found_error) {
    const Json::Value & errorObject = packet["error"];
    if (!errorObject.isObject()) {
      DEBUGOUT("validateResponse") "Invalid: Error member is not object.";
      return false;
    }

    // "code" must be an integer
    if (!errorObject["code"].isIntegral()) {
      DEBUGOUT("validateResponse") "Invalid: Error code is not integral.";
      return false;
    }

    // "message" must be a string
    if (!errorObject["message"].isString()) {
      DEBUGOUT("validateResponse") "Invalid: Error message is invalid.";
      return false;
    }
  }

  // "id" must be a string, a number, or null, but should not be null or
  // fractional
  const Json::Value & idValue = packet["id"];
  if (!idValue.isString() && !idValue.isNumeric() && !idValue.isNull()) {
    DEBUGOUT("validateResponse") "Invalid: id value must be a string, a number, "
        "or null.";
    return false;
  }
  else if (idValue.isNumeric() && !idValue.isIntegral()) {
    DEBUGOUT("validateResponse") "Caution: 'id' should be integral if numeric.";
  }
  else if (idValue.isNull()) {
    DEBUGOUT("validateResponse") "Caution: 'id' should not be null.";
  }

  // Print extra members
  if (extraMembers.size() != 0) {
    if (this->m_debug) {
      std::string extra;
      for (Json::Value::Members::const_iterator it = extraMembers.begin(),
           it_end = extraMembers.end(); it != it_end; ++it) {
        if (it != extraMembers.begin())
          extra += ", ";
        extra += *it;
      }
      DEBUGOUT("validateResponse") "Warn: Extra top-level members found:"
          << extra.c_str();
    }

    if (strict)
      return false;
  }


  return true;
}

bool JsonRpc::validateNotification(const mqPacketType &packet, bool strict)
{
  Json::Value root;
  Json::Reader reader;
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  return this->validateNotification(root, strict);
}

bool JsonRpc::validateNotification(const Json::Value &packet, bool strict)
{
  if (!packet.isObject()) {
    DEBUGOUT("validateNotification") "Invalid: Root node is not an object!";
    return false;
  }

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

  if (!found_jsonrpc) {
    DEBUGOUT("validateNotification") "Warn: 'jsonrpc' not found!";
    if (strict)
      return false;
  }
  if (!found_method) {
    DEBUGOUT("validateNotification") "Invalid: 'method' not found!";
    return false;
  }
// Params are optional.
//  if (!found_params) {
//    DEBUGOUT("validateNotification") "Invalid: 'params' not found!";
//    return false;
//  }
  if (found_id) {
    DEBUGOUT("validateNotification") "Invalid: 'id' found!";
    return false;
  }

  // Validate objects
  // "method" must be a string
  if (!packet["method"].isString()) {
    DEBUGOUT("validateNotification") "Invalid: 'method' is not a string!";
    return false;
  }
  // "params" may be omitted, but must be structured if present
  if (found_params) {
    if (!packet["params"].isObject() && !packet["params"].isArray()) {
      DEBUGOUT("validateNotification") "Invalid: 'params' must be either an array "
          "or an object!";
      return false;
    }
  }

  // Print extra members
  if (extraMembers.size() != 0) {
    if (this->m_debug) {
      std::string extra;
      for (Json::Value::Members::const_iterator it = extraMembers.begin(),
           it_end = extraMembers.end(); it != it_end; ++it) {
        if (it != extraMembers.begin())
          extra += ", ";
        extra += *it;
      }
      DEBUGOUT("validateNotification") "Warn: Extra top-level members found:"
          << extra.c_str();
    }

    if (strict)
      return false;
  }

  return true;
}

Json::Value JsonRpc::generateEmptyRequest(mqIdType id)
{
  Json::Value ret (Json::objectValue);
  ret["jsonrpc"] = "2.0";
  ret["method"] = Json::nullValue;
  ret["id"] = id;

  return ret;
}

Json::Value JsonRpc::generateEmptyResponse(mqIdType id)
{
  Json::Value ret (Json::objectValue);
  ret["jsonrpc"] = "2.0";
  ret["result"] = Json::nullValue;
  ret["id"] = id;

  return ret;
}

Json::Value JsonRpc::generateEmptyError(mqIdType id)
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

JsonRpc::PacketType JsonRpc::guessPacketType(const Json::Value &root) const
{
  if (!root.isObject()) {
    DEBUGOUT("guessPacketType") "Invalid packet: root node is not an Object.";
    return INVALID_PACKET;
  }

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

  DEBUGOUT("guessPacketType") "Invalid packet: No recognized keys.";
  return INVALID_PACKET;
}

JsonRpc::PacketMethod JsonRpc::guessPacketMethod(const Json::Value &root) const
{
  if (!root.isObject()) {
    DEBUGOUT("guessPacketMethod") "Invalid packet: root node is not an Object.";
    return INVALID_METHOD;
  }

  const Json::Value & methodValue = root["method"];

  if (methodValue != Json::nullValue) {
    if (!methodValue.isString()) {
      DEBUGOUT("guessPacketMethod")
          "Invalid packet: Contains non-string 'method' member.";
      return INVALID_METHOD;
    }

    const char *methodCString = methodValue.asCString();

    if (qstrcmp(methodCString, "listQueues") == 0)
      return LIST_QUEUES;
    else if (qstrcmp(methodCString, "submitJob") == 0)
      return SUBMIT_JOB;
    else if (qstrcmp(methodCString, "cancelJob") == 0)
      return CANCEL_JOB;
    else if (qstrcmp(methodCString, "jobStateChanged") == 0)
      return JOB_STATE_CHANGED;

    DEBUGOUT("guessPacketMethod") "Invalid packet: Contains unrecognized "
        "'method' member:" << methodCString;
    return UNRECOGNIZED_METHOD;
  }

  // No method present -- this is a reply. Determine if it's a reply to this
  // client.
  const Json::Value & idValue = root["id"];

  if (idValue != Json::nullValue) {
    // We will only submit unsigned integral ids
    if (!idValue.isUInt())
      return IGNORE_METHOD;

    mqIdType packetId = static_cast<mqIdType>(idValue.asUInt());

    if (m_pendingRequests.contains(packetId)) {
      return m_pendingRequests[packetId];
    }

    // if the packetId isn't in the lookup table, it's not from us
    return IGNORE_METHOD;
  }

  // No method or id present?
  return INVALID_METHOD;
}

void JsonRpc::handleUnparsablePacket(const mqPacketType &data) const
{
  Json::Value errorData (Json::objectValue);

  errorData["receivedPacket"] = data.constData();

  emit invalidPacketReceived(Json::nullValue, errorData);
}

void JsonRpc::handleInvalidRequest(const Json::Value &root) const
{
  Json::Value errorData (Json::objectValue);

  errorData["receivedJson"] = root;

  emit invalidRequestReceived(root["id"], errorData);
}

void JsonRpc::handleUnrecognizedRequest(const Json::Value &root) const
{
  Json::Value errorData (Json::objectValue);

  errorData["receivedJson"] = root;

  emit invalidRequestReceived(root["id"], errorData);
}

void JsonRpc::handleListQueuesRequest(const Json::Value &root) const
{
  const mqIdType id = static_cast<mqIdType>(root["id"].asLargestUInt());
  emit queueListRequestReceived(id);
}

void JsonRpc::handleListQueuesResult(const Json::Value &root) const
{
  const mqIdType id = static_cast<mqIdType>(root["id"].asLargestUInt());

  const Json::Value &resultObject = root["result"];
  if (!resultObject.isArray()) {
    Json::StyledWriter writer;
    const std::string requestString = writer.write(root);
    qWarning() << "Error: Queue list result is ill-formed:\n"
               << requestString.c_str();
    return;
  }

  // Populate queue list:
  QList<QPair<QString, QStringList> > queueList;
#if QT_VERSION >= QT_VERSION_CHECK(4,7,0)
  queueList.reserve(resultObject.size());
#endif

  // Iterate through queues
  for (Json::Value::const_iterator it = resultObject.begin(),
       it_end = resultObject.end(); it != it_end; ++it) {
    const QString queueName (it.memberName());

    // Extract program data
    const Json::Value &programArray = (*it)["programs"];

    // No programs, just add an empty list
    if (programArray.isNull())
      queueList.append(QPair<QString, QStringList>(queueName, QStringList()));
      continue;

    // Not an array? Add an empty list
    if (!programArray.isArray()) {
      Json::StyledWriter writer;
      const std::string programsString = writer.write(programArray);
      qWarning() << "Error: List of programs for" << queueName
                 << "is ill-formed:\n"
                 << programsString.c_str();
      queueList.append(QPair<QString, QStringList>(queueName, QStringList()));
      continue;
    }

    QStringList programList;
#if QT_VERSION >= QT_VERSION_CHECK(4,7,0)
    programList.reserve(programArray.size());
#endif
    // Iterate through programs
    for (Json::Value::const_iterator pit = programArray.begin(),
         pit_end = programArray.end(); pit != pit_end; ++pit) {
      if ((*pit).isString())
        programList << (*pit).asCString();
    }

    queueList << QPair<QString, QStringList>(queueName, programList);
  }


  emit queueListReceived(id, queueList);
}

void JsonRpc::handleListQueuesError(const Json::Value &root) const
{
  Q_UNUSED(root);
  qWarning() << Q_FUNC_INFO << "is not implemented.";
}

void JsonRpc::handleSubmitJobRequest(const Json::Value &root) const
{
  const mqIdType id = static_cast<mqIdType>(root["id"].asLargestUInt());

  const Json::Value &paramsObject = root["params"];
  if (!paramsObject.isObject()) {
    Json::StyledWriter writer;
    const std::string requestString = writer.write(root);
    qWarning() << "Error: submitJob request is ill-formed:\n"
               << requestString.c_str();
    return;
  }

  // Populate options object:
  QVariantHash optionHash;
  optionHash.reserve(paramsObject.size());

  // Iterate through options
  for (Json::Value::const_iterator it = paramsObject.begin(),
       it_end = paramsObject.end(); it != it_end; ++it) {
    const QString optionName (it.memberName());

    // Extract option data
    QVariant optionVariant;
    switch ((*it).type())
    {
    case Json::nullValue:
      optionVariant = QVariant();
      break;
    case Json::intValue:
      optionVariant = (*it).asLargestInt();
      break;
    case Json::uintValue:
      optionVariant = (*it).asLargestUInt();
      break;
    case Json::realValue:
      optionVariant = (*it).asDouble();
      break;
    case Json::stringValue:
      optionVariant = (*it).asCString();
      break;
    case Json::booleanValue:
      optionVariant = (*it).asBool();
      break;
    default:
    case Json::arrayValue:
    case Json::objectValue: {
      Json::StyledWriter writer;
      const std::string valueString = writer.write(*it);
      qWarning() << "Unsupported option type encountered (option name:"
                 << optionName << ")\n" << valueString.c_str();
      optionVariant = QVariant();
      break;
    }
    }

    optionHash.insert(optionName, optionVariant);
  }


  emit jobSubmissionRequestReceived(id, optionHash);
}

void JsonRpc::handleSubmitJobResult(const Json::Value &root) const
{
  const mqIdType id = static_cast<mqIdType>(root["id"].asLargestUInt());

  const Json::Value &resultObject = root["result"];

  mqIdType moleQueueId;
  mqIdType jobId;
  QDir workingDirectory;

  if (!resultObject["moleQueueId"].isUInt() ||
      !resultObject["queueJobId"].isUInt() ||
      !resultObject["workingDirectory"].isString()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job submission result is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  moleQueueId = static_cast<mqIdType>(
        resultObject["moleQueueId"].asLargestUInt());
  jobId = static_cast<mqIdType>(resultObject["queueJobId"].asLargestUInt());
  workingDirectory = QDir(QString(
                            resultObject["workingDirectory"].asCString()));

  if (!workingDirectory.exists())
    qWarning() << "Warning: Working directory '"
               << workingDirectory.absolutePath() << "' for MoleQueue job id"
               << moleQueueId << "does not exist.";

  emit successfulSubmissionReceived(id, moleQueueId, jobId, workingDirectory);
}

void JsonRpc::handleSubmitJobError(const Json::Value &root) const
{
  const mqIdType id = static_cast<mqIdType>(root["id"].asLargestUInt());

  if (!root["error"]["code"].isIntegral() ||
      !root["error"]["message"].isString()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job submission failure response is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  const JobSubmissionErrorCode errorCode = static_cast<JobSubmissionErrorCode>(
        root["error"]["code"].asLargestInt());

  const QString errorMessage (root["error"]["message"].asCString());

  emit failedSubmissionReceived(id, errorCode, errorMessage);
}

void JsonRpc::handleCancelJobRequest(const Json::Value &root) const
{
  const mqIdType id = static_cast<mqIdType>(root["id"].asLargestUInt());

  const Json::Value &paramsObject = root["params"];

  if (!paramsObject.isObject() ||
      !paramsObject["moleQueueJobId"].isUInt()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job cancellation request is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  const mqIdType moleQueueJobId = static_cast<mqIdType>(
        paramsObject["moleQueueJobId"].asLargestUInt());

  emit jobCancellationRequestReceived(id, moleQueueJobId);
}

void JsonRpc::handleCancelJobResult(const Json::Value &root) const
{
  const mqIdType id = static_cast<mqIdType>(root["id"].asLargestUInt());

  const Json::Value &resultObject = root["result"];

  mqIdType moleQueueId;

  if (!resultObject.isUInt()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job cancellation result is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  moleQueueId = static_cast<mqIdType>(resultObject.asLargestUInt());

  emit jobCancellationRequestReceived(id, moleQueueId);
}

void JsonRpc::handleCancelJobError(const Json::Value &root) const
{
  Q_UNUSED(root);
  qWarning() << Q_FUNC_INFO << "is not implemented.";
}

void JsonRpc::handleJobStateChangedNotification(const Json::Value &root) const
{
  const Json::Value &paramsObject = root["params"];

  mqIdType moleQueueId;
  JobState oldState;
  JobState newState;

  if (!paramsObject.isObject() ||
      !paramsObject["moleQueueJobId"].isUInt() ||
      !paramsObject["oldState"].isString() ||
      !paramsObject["newState"].isString() ){
    Json::StyledWriter writer;
    const std::string requestString = writer.write(root);
    qWarning() << "Job cancellation result is ill-formed:\n"
               << requestString.c_str();
    return;
  }

  moleQueueId = static_cast<mqIdType>(
        paramsObject["moleQueueJobId"].asLargestUInt());
  oldState = stringToJobState(paramsObject["oldState"].asCString());
  newState = stringToJobState(paramsObject["newState"].asCString());

  emit jobStateChangeReceived(moleQueueId, oldState, newState);
}

void JsonRpc::registerRequest(mqIdType packetId,
                              JsonRpc::PacketMethod method)
{
  DEBUGOUT("registerRequest")
      "New request -- packetId:" << packetId << "method:" << method;

  m_pendingRequests[packetId] = method;
}

void JsonRpc::registerReply(mqIdType packetId)
{
  DEBUGOUT("registerReply") "New reply -- packetId:" << packetId;
  m_pendingRequests.remove(packetId);
}

} // end namespace MoleQueue
