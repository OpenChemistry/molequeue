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

#include "serverjsonrpc.h"

#include "job.h"
#include "transport/qtjson.h"

#include <json/json.h>

namespace MoleQueue {

ServerJsonRpc::ServerJsonRpc(QObject *p) :
  MoleQueue::JsonRpc(p)
{
}

ServerJsonRpc::~ServerJsonRpc()
{
}

PacketType
ServerJsonRpc::generateJobSubmissionConfirmation(IdType moleQueueId,
                                                 const QString &workingDir,
                                                 const MessageIdType &packetId)
{
  Json::Value packet = generateEmptyResponse(packetId);

  Json::Value resultObject(Json::objectValue);
  resultObject["moleQueueId"] = moleQueueId;
  resultObject["workingDirectory"] = workingDir.toStdString();

  packet["result"] = resultObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret(ret_stdstr.c_str());

  return ret;
}

PacketType ServerJsonRpc::generateJobCancellationConfirmation(
    IdType moleQueueId, const MessageIdType &packetId)
{
  Json::Value packet = generateEmptyResponse(packetId);

  packet["result"] = moleQueueId;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret(ret_stdstr.c_str());

  return ret;
}

PacketType ServerJsonRpc::generateJobCancellationError(
    ErrorCode errorCode, const QString &message,
    IdType moleQueueId, const MessageIdType &packetId)
{
  Json::Value packet = generateEmptyError(packetId);

  packet["error"]["code"]    = errorCode;
  packet["error"]["message"] = message.toStdString();
  packet["error"]["data"]    = moleQueueId;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret (ret_stdstr.c_str());

  return ret;
}

PacketType
ServerJsonRpc::generateLookupJobResponse(const Job &req,
                                         IdType moleQueueId,
                                         const MessageIdType &packetId)
{
  Json::Value packet;

  if (!req.isValid()) {
    packet = generateEmptyError(packetId);
    Json::Value errorObject(Json::objectValue);
    errorObject["message"] = "Unknown MoleQueue ID";
    errorObject["code"] = 0;
    errorObject["data"] = moleQueueId;
    packet["error"] = errorObject;
  }
  else {
    packet = generateEmptyResponse(packetId);
    packet["result"] = QtJson::toJson(req.hash());
  }

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret(ret_stdstr.c_str());

  return ret;
}

PacketType ServerJsonRpc::generateQueueList(const QueueListType &queueList,
                                            const MessageIdType &packetId)
{
  Json::Value packet = generateEmptyResponse(packetId);

  Json::Value resultObject(Json::objectValue);
  foreach (const QString queueName, queueList.keys()) {

    Json::Value programArray(Json::arrayValue);
    foreach (const QString prog, queueList[queueName]) {
      const std::string progName = prog.toStdString();
      programArray.append(progName);
    }
    resultObject[queueName.toStdString()] = programArray;
  }

  packet["result"] = resultObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret(ret_stdstr.c_str());

  return ret;
}

PacketType ServerJsonRpc::generateJobStateChangeNotification(IdType moleQueueId,
                                                             JobState oldState,
                                                             JobState newState)
{
  Json::Value packet = generateEmptyNotification();

  packet["method"] = "jobStateChanged";

  Json::Value paramsObject(Json::objectValue);
  paramsObject["moleQueueId"]         = moleQueueId;
  paramsObject["oldState"]            = jobStateToString(oldState);
  paramsObject["newState"]            = jobStateToString(newState);

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret(ret_stdstr.c_str());

  return ret;
}

int ServerJsonRpc::mapMethodNameToInt(const QString &methodName) const
{
  if (methodName == "listQueues")
    return LIST_QUEUES;
  else if (methodName == "submitJob")
    return SUBMIT_JOB;
  else if (methodName == "cancelJob")
    return CANCEL_JOB;
  else if (methodName == "lookupJob")
    return LOOKUP_JOB;
  else if (methodName == "jobStateChanged")
    return JOB_STATE_CHANGED;

  return UNRECOGNIZED_METHOD;
}

void ServerJsonRpc::handleMessage(int method, const Message &msg)
{
  switch (method) {
  case LIST_QUEUES:
  {
    switch (msg.type()) {
    default:
    case Message::INVALID_MESSAGE:
    case Message::NOTIFICATION_MESSAGE:
    case Message::RESULT_MESSAGE:
    case Message::ERROR_MESSAGE:
      handleInvalidRequest(msg);
      break;
    case Message::REQUEST_MESSAGE:
      handleListQueuesRequest(msg);
      break;
    }
    break;
  }
  case SUBMIT_JOB:
  {
    switch (msg.type()) {
    default:
    case Message::INVALID_MESSAGE:
    case Message::NOTIFICATION_MESSAGE:
    case Message::RESULT_MESSAGE:
    case Message::ERROR_MESSAGE:
      handleInvalidRequest(msg);
      break;
    case Message::REQUEST_MESSAGE:
      handleSubmitJobRequest(msg);
      break;
    }
    break;
  }
  case CANCEL_JOB:
  {
    switch (msg.type()) {
    default:
    case Message::INVALID_MESSAGE:
    case Message::NOTIFICATION_MESSAGE:
    case Message::RESULT_MESSAGE:
    case Message::ERROR_MESSAGE:
      handleInvalidRequest(msg);
      break;
    case Message::REQUEST_MESSAGE:
      handleCancelJobRequest(msg);
      break;
    }
    break;
  }
  case LOOKUP_JOB:
  {
    switch (msg.type()) {
    default:
    case Message::INVALID_MESSAGE:
    case Message::NOTIFICATION_MESSAGE:
    case Message::RESULT_MESSAGE:
    case Message::ERROR_MESSAGE:
      handleInvalidRequest(msg);
      break;
    case Message::REQUEST_MESSAGE:
      handleLookupJobRequest(msg);
      break;
    }
    break;
  }
  case JOB_STATE_CHANGED:
  {
    handleInvalidRequest(msg);
    break;
  }
  default:
    handleInvalidRequest(msg);
    break;
  }

}

void ServerJsonRpc::handleListQueuesRequest(const Message &msg) const
{
  emit queueListRequestReceived(msg);
}

void ServerJsonRpc::handleSubmitJobRequest(const Message &msg) const
{
  const Json::Value &paramsObject = msg.json()["params"];
  if (!paramsObject.isObject()) {
    qWarning() << "Error: submitJob request is ill-formed:\n"
               << msg.data();
    return;
  }

  // Populate options object:
  QVariantHash optionHash = QtJson::toVariant(paramsObject).toHash();

  emit jobSubmissionRequestReceived(msg, optionHash);
}

void ServerJsonRpc::handleCancelJobRequest(const Message &msg) const
{
  const Json::Value &paramsObject = msg.json()["params"];

  if (!paramsObject.isObject() ||
      !paramsObject["moleQueueId"].isIntegral()) {
    qWarning() << "Job cancellation request is ill-formed:\n"
               << msg.data();
    return;
  }

  const IdType moleQueueId = static_cast<IdType>(
        paramsObject["moleQueueId"].asLargestUInt());

  emit jobCancellationRequestReceived(msg, moleQueueId);
}

void ServerJsonRpc::handleLookupJobRequest(const Message &msg) const
{
  const Json::Value &paramsObject = msg.json()["params"];

  if (!paramsObject.isObject() ||
      !paramsObject["moleQueueId"].isIntegral()) {
    qWarning() << "Job lookup request is ill-formed:\n"
               << msg.data();
    return;
  }

  const IdType moleQueueId = static_cast<IdType>(
        paramsObject["moleQueueId"].asLargestUInt());

  emit lookupJobRequestReceived(msg, moleQueueId);
}

} // namespace MoleQueue
