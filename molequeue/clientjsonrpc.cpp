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

#include "clientjsonrpc.h"

#include "job.h"
#include "transport/qtjson.h"

#include <json/json.h>

#include <QtCore/QDir>

namespace MoleQueue {

ClientJsonRpc::ClientJsonRpc(QObject *p) :
  MoleQueue::JsonRpc(p)
{
}

ClientJsonRpc::~ClientJsonRpc()
{
}

PacketType ClientJsonRpc::generateJobRequest(const Job &job,
                                             const MessageIdType &packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "submitJob";

  Json::Value paramsObject = QtJson::toJson(job.hash());

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret(ret_stdstr.c_str());

  registerRequest(packetId, SUBMIT_JOB);

  return ret;
}

PacketType ClientJsonRpc::generateJobCancellation(const Job &req,
                                                  const MessageIdType &packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "cancelJob";

  Json::Value paramsObject(Json::objectValue);
  paramsObject["moleQueueId"] = req.moleQueueId();

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret(ret_stdstr.c_str());

  registerRequest(packetId, CANCEL_JOB);

  return ret;
}

PacketType ClientJsonRpc::generateLookupJobRequest(
    IdType moleQueueId, const MessageIdType &packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "lookupJob";

  Json::Value paramsObject(Json::objectValue);
  paramsObject["moleQueueId"] = moleQueueId;

  packet["params"] = paramsObject;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret(ret_stdstr.c_str());

  registerRequest(packetId, LOOKUP_JOB);

  return ret;
}

PacketType
ClientJsonRpc::generateQueueListRequest(const MessageIdType &packetId)
{
  Json::Value packet = generateEmptyRequest(packetId);

  packet["method"] = "listQueues";

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret(ret_stdstr.c_str());

  registerRequest(packetId, LIST_QUEUES);

  return ret;
}

int ClientJsonRpc::mapMethodNameToInt(const QString &methodName) const
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

void ClientJsonRpc::handleMessage(int method, const Message &msg)
{
  switch (method) {
  case LIST_QUEUES:
  {
    switch (msg.type()) {
    default:
    case Message::INVALID_MESSAGE:
    case Message::NOTIFICATION_MESSAGE:
    case Message::ERROR_MESSAGE:
    case Message::REQUEST_MESSAGE:
      handleInvalidRequest(msg);
      break;
    case Message::RESULT_MESSAGE:
      handleListQueuesResult(msg);
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
    case Message::REQUEST_MESSAGE:
      handleInvalidRequest(msg);
      break;
    case Message::RESULT_MESSAGE:
      handleSubmitJobResult(msg);
      break;
    case Message::ERROR_MESSAGE:
      handleSubmitJobError(msg);
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
    case Message::REQUEST_MESSAGE:
      handleInvalidRequest(msg);
      break;
    case Message::RESULT_MESSAGE:
      handleCancelJobResult(msg);
      break;
    case Message::ERROR_MESSAGE:
      handleCancelJobError(msg);
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
    case Message::REQUEST_MESSAGE:
      handleInvalidRequest(msg);
      break;
    case Message::RESULT_MESSAGE:
      handleLookupJobResult(msg);
      break;
    case Message::ERROR_MESSAGE:
      handleLookupJobError(msg);
      break;
    }
    break;
  }
  case JOB_STATE_CHANGED:
  {
    switch (msg.type()) {
    default:
    case Message::INVALID_MESSAGE:
    case Message::REQUEST_MESSAGE:
    case Message::RESULT_MESSAGE:
    case Message::ERROR_MESSAGE:
      handleInvalidRequest(msg);
      break;
    case Message::NOTIFICATION_MESSAGE:
      handleJobStateChangedNotification(msg);
      break;
    }
    break;
  }
  default:
    handleInvalidRequest(msg);
    break;
  }
}

void ClientJsonRpc::handleListQueuesResult(const Message &msg) const
{
  const Json::Value &resultObject = msg.json()["result"];
  if (!resultObject.isObject()) {
    qWarning() << "Error: Queue list result is ill-formed:\n"
               << msg.data();
    return;
  }

  // Populate queue list:
  QueueListType queueList;
  queueList.reserve(resultObject.size());

  // Iterate through queues
  for (Json::Value::const_iterator it = resultObject.begin(),
       it_end = resultObject.end(); it != it_end; ++it) {
    const QString queueName(it.memberName());

    // Extract program data
    const Json::Value &programArray = *it;

    // No programs, just add an empty list
    if (programArray.isNull()) {
      queueList.insert(queueName, QStringList());
      continue;
    }

    // Not an array? Add an empty list
    if (!programArray.isArray()) {
      Json::StyledWriter writer;
      const std::string programsString = writer.write(programArray);
      qWarning() << "Error: List of programs for" << queueName
                 << "is ill-formed:\n"
                 << programsString.c_str();
      queueList.insert(queueName, QStringList());
      continue;
    }

    QStringList programList;
    programList.reserve(programArray.size());
    // Iterate through programs
    for (Json::Value::const_iterator pit = programArray.begin(),
         pit_end = programArray.end(); pit != pit_end; ++pit) {
      if ((*pit).isString())
        programList << (*pit).asCString();
    }

    queueList.insert(queueName, programList);
  }

  emit queueListReceived(msg.id(), queueList);
}

void ClientJsonRpc::handleSubmitJobResult(const Message &msg) const
{
  const Json::Value &resultObject = msg.json()["result"];

  IdType moleQueueId;
  QDir workingDirectory;

  if (!resultObject["moleQueueId"].isIntegral() ||
      !resultObject["workingDirectory"].isString()) {
    qWarning() << "Job submission result is ill-formed:\n"
               << msg.data();
    return;
  }

  moleQueueId = static_cast<IdType>(
        resultObject["moleQueueId"].asLargestUInt());
  workingDirectory = QDir(QString(
                            resultObject["workingDirectory"].asCString()));

  emit successfulSubmissionReceived(msg.id(), moleQueueId, workingDirectory);
}

void ClientJsonRpc::handleSubmitJobError(const Message &msg) const
{
  if (!msg.json()["error"]["code"].isIntegral() ||
      !msg.json()["error"]["message"].isString()) {
    qWarning() << "Job submission failure response is ill-formed:\n"
               << msg.data();
    return;
  }

  const ErrorCode errorCode = static_cast<ErrorCode>(
        msg.json()["error"]["code"].asLargestInt());

  const QString errorMessage(msg.json()["error"]["message"].asCString());

  emit failedSubmissionReceived(msg.id(), errorCode, errorMessage);
}

void ClientJsonRpc::handleCancelJobResult(const Message &msg) const
{
  const Json::Value &resultObject = msg.json()["result"];

  IdType moleQueueId;

  if (!resultObject.isIntegral()) {
    qWarning() << "Job cancellation result is ill-formed:\n"
               << msg.data();
    return;
  }

  moleQueueId = static_cast<IdType>(resultObject.asLargestUInt());

  emit jobCancellationConfirmationReceived(msg.id(), moleQueueId);
}

void ClientJsonRpc::handleCancelJobError(const Message &msg) const
{
  if (!msg.json()["error"]["code"].isIntegral() ||
      !msg.json()["error"]["data"].isIntegral() ||
      !msg.json()["error"]["message"].isString()) {
    qWarning() << "Job lookup failure response is ill-formed:\n"
               << msg.data();
    return;
  }

  const ErrorCode errorCode =
      static_cast<ErrorCode>(
        msg.json()["error"]["code"].asLargestUInt());
  const QString message(msg.json()["error"]["message"].asCString());
  const IdType moleQueueId =
      static_cast<IdType>(msg.json()["error"]["data"].asLargestUInt());

  emit jobCancellationErrorReceived(msg.id(), moleQueueId, errorCode, message);
}

void ClientJsonRpc::handleLookupJobResult(const Message &msg) const
{
  const Json::Value &resultObject = msg.json()["result"];

  if (!resultObject.isObject()) {
    qWarning() << "Job lookup result is ill-formed:\n"
               << msg.data();
    return;
  }

  QVariantHash hash = QtJson::toVariant(resultObject).toHash();

  emit lookupJobResponseReceived(msg.id(), hash);
}

void ClientJsonRpc::handleLookupJobError(const Message &msg) const
{
  if (!msg.json()["error"]["code"].isIntegral() ||
      !msg.json()["error"]["data"].isIntegral() ||
      !msg.json()["error"]["message"].isString()) {
    qWarning() << "Job lookup failure response is ill-formed:\n"
               << msg.data();
    return;
  }

  const IdType moleQueueId =
      static_cast<IdType>(msg.json()["error"]["data"].asLargestUInt());

  emit lookupJobErrorReceived(msg.id(), moleQueueId);
}

void ClientJsonRpc::handleJobStateChangedNotification(const Message &msg) const
{
  const Json::Value &paramsObject = msg.json()["params"];

  IdType moleQueueId;
  JobState oldState;
  JobState newState;

  if (!paramsObject.isObject() ||
      !paramsObject["moleQueueId"].isIntegral() ||
      !paramsObject["oldState"].isString() ||
      !paramsObject["newState"].isString() ){
    qWarning() << "Job cancellation result is ill-formed:\n"
               << msg.data();
    return;
  }

  moleQueueId = static_cast<IdType>(
        paramsObject["moleQueueId"].asLargestUInt());
  oldState = stringToJobState(paramsObject["oldState"].asCString());
  newState = stringToJobState(paramsObject["newState"].asCString());

  emit jobStateChangeReceived(moleQueueId, oldState, newState);
}

} // namespace MoleQueue
