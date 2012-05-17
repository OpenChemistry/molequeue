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

#include <json/json.h>

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
  qRegisterMetaType<QDir>("QDir");
  qRegisterMetaType<Json::Value>("Json::Value");
  qRegisterMetaType<IdType>("IdType");
  qRegisterMetaType<JobState>("JobState");
  qRegisterMetaType<QueueListType>("QueueListType");
  qRegisterMetaType<JobSubmissionErrorCode>("JobSubmissionErrorCode");
}

JsonRpc::~JsonRpc()
{
  if (m_debug && m_pendingRequests.size() != 0) {
    DEBUGOUT("~JsonRpc") "Dangling requests upon destruction:";
    for (QHash<IdType, PacketMethod>::const_iterator
         it     = m_pendingRequests.constBegin(),
         it_end = m_pendingRequests.constEnd(); it != it_end; ++it) {
      DEBUGOUT("~JsonRpc") "    PacketId:" << it.key() << "Request Method:"
                                           << it.value();
    }
  }
}

PacketType JsonRpc::generateJobRequest(const JobRequest &req,
                                         IdType packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "submitJob";

  Json::Value paramsObject (Json::objectValue);

  const QVariantHash reqHash = req.hash();

  for (QVariantHash::const_iterator it = reqHash.constBegin(),
       it_end = reqHash.constEnd(); it != it_end; ++it) {
    Json::Value val;
    switch (it.value().type()) {
    case QVariant::Bool:
      val = it.value().toBool();
      break;
    case QVariant::Int:
      val = it.value().toInt();
      break;
    case QVariant::LongLong:
      val = it.value().toLongLong();
      break;
    case QVariant::UInt:
      val = it.value().toUInt();
      break;
    case QVariant::ULongLong:
      val = it.value().toULongLong();
      break;
    case QVariant::Double:
      val = it.value().toDouble();
      break;
    case QVariant::String: {
      const std::string str = it.value().toString().toStdString();
      val = str;
      break;
    }
    case QVariant::ByteArray:
      val = it.value().toByteArray().constData();
      break;
    default:
    case QVariant::Invalid:
    case QVariant::Map:
    case QVariant::List:
    case QVariant::Char:
    case QVariant::StringList:
    case QVariant::BitArray:
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
    case QVariant::Url:
    case QVariant::Locale:
    case QVariant::Rect:
    case QVariant::RectF:
    case QVariant::Size:
    case QVariant::SizeF:
    case QVariant::Line:
    case QVariant::LineF:
    case QVariant::Point:
    case QVariant::PointF:
    case QVariant::RegExp:
    case QVariant::Hash:
    case QVariant::EasingCurve:
    case QVariant::Font:
    case QVariant::Pixmap:
    case QVariant::Brush:
    case QVariant::Color:
    case QVariant::Palette:
    case QVariant::Icon:
    case QVariant::Image:
    case QVariant::Polygon:
    case QVariant::Region:
    case QVariant::Bitmap:
    case QVariant::Cursor:
    case QVariant::SizePolicy:
    case QVariant::KeySequence:
    case QVariant::Pen:
    case QVariant::TextLength:
    case QVariant::TextFormat:
    case QVariant::Matrix:
    case QVariant::Transform:
    case QVariant::Matrix4x4:
    case QVariant::Vector2D:
    case QVariant::Vector3D:
    case QVariant::Vector4D:
    case QVariant::Quaternion:
    case QVariant::UserType:
    case QVariant::LastType:
      DEBUGOUT("generateJobRequest") "Unhandled type in hash:"
          << it.value().type();
      continue;
    } // end switch

    const std::string name = it.key().toStdString();
    paramsObject[name] = val;
  } // end for

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobRequest") "New job request:\n" << ret;

  this->registerRequest(packetId, SUBMIT_JOB);

  return ret;
}

PacketType
JsonRpc::generateJobSubmissionConfirmation(IdType moleQueueJobId,
                                           IdType queueJobId,
                                           const QString &workingDirectory,
                                           IdType packetId)
{
  Json::Value packet = generateEmptyResponse(packetId);

  Json::Value resultObject (Json::objectValue);
  resultObject["moleQueueJobId"] = moleQueueJobId;
  resultObject["queueJobId"] = queueJobId;
  resultObject["workingDirectory"] = workingDirectory.toStdString();

  packet["result"] = resultObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobSubmissionConfirmation")
      "New job confirmation generated:\n" << ret;

  return ret;
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

  DEBUGOUT("generateErrorResponse")
      "New error response generated:\n" << ret;

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

  DEBUGOUT("generateErrorResponse")
      "New error response generated:\n" << ret;

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

  DEBUGOUT("generateErrorResponse")
      "New error response generated:\n" << ret;

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

  DEBUGOUT("generateErrorResponse")
      "New error response generated:\n" << ret;

  return ret;
}

PacketType JsonRpc::generateJobCancellation(const JobRequest &req,
                                              IdType packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "cancelJob";

  Json::Value paramsObject (Json::objectValue);
  paramsObject["moleQueueJobId"] = req.moleQueueId();

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobCancellation") "New job cancellation request:\n" << ret;

  this->registerRequest(packetId, CANCEL_JOB);

  return ret;
}

PacketType JsonRpc::generateJobCancellationConfirmation(IdType moleQueueId,
                                                          IdType packetId)
{
  Json::Value packet = generateEmptyResponse(packetId);

  packet["result"] = moleQueueId;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobCancellationConfirmation")
      "New job cancellation confirmation generated:\n" << ret;

  return ret;
}

PacketType JsonRpc::generateQueueListRequest(IdType packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "listQueues";

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateQueueListRequest") "New queue list request:\n" << ret;

  this->registerRequest(packetId, LIST_QUEUES);

  return ret;
}

PacketType JsonRpc::generateQueueList(const QueueManager *qmanager,
                                        IdType packetId)
{
  if (!qmanager) {
    qDebug() << Q_FUNC_INFO << "called with a NULL QueueManager?";
    return PacketType();
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
  PacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateQueueList") "Queue list generated:\n" << ret;

  return ret;
}

PacketType
JsonRpc::generateJobStateChangeNotification(IdType moleQueueJobId,
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
  PacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobStateChangeNotification")
      "New state change:\n" << ret;

  return ret;
}

void JsonRpc::interpretIncomingPacket(const PacketType &packet)
{
  // Read packet into a Json value
  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(packet.constData(), packet.constData() + packet.size(),
                    root, false)) {
    this->handleUnparsablePacket(packet);
    return;
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
    return;
  }

  PacketForm   form   = this->guessPacketForm(data);
  PacketMethod method = this->guessPacketMethod(data);

  // Validate detected type
  switch (form) {
  case REQUEST_PACKET:
    if (!this->validateRequest(data, false))
      form = INVALID_PACKET;
    break;
  case RESULT_PACKET:
  case ERROR_PACKET:
    if (!this->validateResponse(data, false))
      form = INVALID_PACKET;
    break;
  case NOTIFICATION_PACKET:
    if (!this->validateNotification(data, false))
      form = INVALID_PACKET;
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
    switch (form) {
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
    switch (form) {
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
    switch (form) {
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
    switch (form) {
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
  if (form == RESULT_PACKET || form == ERROR_PACKET)
    registerReply(static_cast<IdType>(data["id"].asLargestUInt()));

}

bool JsonRpc::validateRequest(const PacketType &packet, bool strict)
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

bool JsonRpc::validateResponse(const PacketType &packet, bool strict)
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

bool JsonRpc::validateNotification(const PacketType &packet, bool strict)
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

void JsonRpc::handleUnparsablePacket(const PacketType &data) const
{
  Json::Value errorData (Json::objectValue);

  errorData["receivedPacket"] = data.constData();

  emit invalidPacketReceived(Json::nullValue, errorData);
}

void JsonRpc::handleInvalidRequest(const Json::Value &root) const
{
  Json::Value errorData (Json::objectValue);

  errorData["receivedJson"] = Json::Value(root);

  emit invalidRequestReceived((root.isObject()) ? root["id"] : Json::nullValue,
                              errorData);
}

void JsonRpc::handleUnrecognizedRequest(const Json::Value &root) const
{
  Json::Value errorData (Json::objectValue);

  errorData["receivedJson"] = root;

  emit unrecognizedRequestReceived(root["id"], errorData);
}

void JsonRpc::handleListQueuesRequest(const Json::Value &root) const
{
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());
  emit queueListRequestReceived(id);
}

void JsonRpc::handleListQueuesResult(const Json::Value &root) const
{
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

  const Json::Value &resultObject = root["result"];
  if (!resultObject.isObject()) {
    Json::StyledWriter writer;
    const std::string requestString = writer.write(root);
    qWarning() << "Error: Queue list result is ill-formed:\n"
               << requestString.c_str();
    return;
  }

  // Populate queue list:
  QueueListType queueList;
#if QT_VERSION >= QT_VERSION_CHECK(4,7,0)
  queueList.reserve(resultObject.size());
#endif

  // Iterate through queues
  for (Json::Value::const_iterator it = resultObject.begin(),
       it_end = resultObject.end(); it != it_end; ++it) {
    const QString queueName (it.memberName());

    // Extract program data
    const Json::Value &programArray = *it;

    // No programs, just add an empty list
    if (programArray.isNull()) {
      queueList.append(QPair<QString, QStringList>(queueName, QStringList()));
      continue;
    }

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
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

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
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

  const Json::Value &resultObject = root["result"];

  IdType moleQueueId;
  IdType jobId;
  QDir workingDirectory;

  if (!resultObject["moleQueueJobId"].isIntegral() ||
      !resultObject["queueJobId"].isIntegral() ||
      !resultObject["workingDirectory"].isString()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job submission result is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  moleQueueId = static_cast<IdType>(
        resultObject["moleQueueJobId"].asLargestUInt());
  jobId = static_cast<IdType>(resultObject["queueJobId"].asLargestUInt());
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
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

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
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

  const Json::Value &paramsObject = root["params"];

  if (!paramsObject.isObject() ||
      !paramsObject["moleQueueJobId"].isIntegral()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job cancellation request is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  const IdType moleQueueJobId = static_cast<IdType>(
        paramsObject["moleQueueJobId"].asLargestUInt());

  emit jobCancellationRequestReceived(id, moleQueueJobId);
}

void JsonRpc::handleCancelJobResult(const Json::Value &root) const
{
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

  const Json::Value &resultObject = root["result"];

  IdType moleQueueId;

  if (!resultObject.isIntegral()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job cancellation result is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  moleQueueId = static_cast<IdType>(resultObject.asLargestUInt());

  emit jobCancellationConfirmationReceived(id, moleQueueId);
}

void JsonRpc::handleCancelJobError(const Json::Value &root) const
{
  Q_UNUSED(root);
  qWarning() << Q_FUNC_INFO << "is not implemented.";
}

void JsonRpc::handleJobStateChangedNotification(const Json::Value &root) const
{
  const Json::Value &paramsObject = root["params"];

  IdType moleQueueId;
  JobState oldState;
  JobState newState;

  if (!paramsObject.isObject() ||
      !paramsObject["moleQueueJobId"].isIntegral() ||
      !paramsObject["oldState"].isString() ||
      !paramsObject["newState"].isString() ){
    Json::StyledWriter writer;
    const std::string requestString = writer.write(root);
    qWarning() << "Job cancellation result is ill-formed:\n"
               << requestString.c_str();
    return;
  }

  moleQueueId = static_cast<IdType>(
        paramsObject["moleQueueJobId"].asLargestUInt());
  oldState = stringToJobState(paramsObject["oldState"].asCString());
  newState = stringToJobState(paramsObject["newState"].asCString());

  emit jobStateChangeReceived(moleQueueId, oldState, newState);
}

void JsonRpc::registerRequest(IdType packetId,
                              JsonRpc::PacketMethod method)
{
  DEBUGOUT("registerRequest")
      "New request -- packetId:" << packetId << "method:" << method;

  m_pendingRequests[packetId] = method;
}

void JsonRpc::registerReply(IdType packetId)
{
  DEBUGOUT("registerReply") "New reply -- packetId:" << packetId;
  m_pendingRequests.remove(packetId);
}

} // end namespace MoleQueue
