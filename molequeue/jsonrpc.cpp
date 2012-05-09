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

mqPacketType JsonRpc::generateJobCancellation(const JobRequest &req,
                                              mqIdType packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "cancelJob";

  Json::Value paramsObject (Json::objectValue);
  paramsObject["molequeue id"] = req.molequeueId();

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobCancellation") "New job cancellation request:\n" << ret;

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
  paramsObject["molequeue id"]        = moleQueueJobId;
  paramsObject["oldState"]            = static_cast<int>(oldState);
  paramsObject["newState"]            = static_cast<int>(newState);

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  mqPacketType ret (ret_stdstr.c_str());

  DEBUGOUT("generateJobStateChangeNotification")
      "New state change:\n" << ret;

  return ret;
}

QVector<mqIdType> JsonRpc::interpretIncomingPacket(const mqPacketType &packet)
{
  // TODO
  return QVector<mqIdType>();
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

Json::Value JsonRpc::generateEmptyNotification()
{
  Json::Value ret (Json::objectValue);
  ret["jsonrpc"] = "2.0";
  ret["method"] = Json::nullValue;

  return ret;
}

} // end namespace MoleQueue
