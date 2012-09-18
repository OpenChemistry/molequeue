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
                                             IdType packetId)
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
                                                  IdType packetId)
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

PacketType ClientJsonRpc::generateLookupJobRequest(IdType moleQueueId,
                                                   IdType packetId)
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

PacketType ClientJsonRpc::generateQueueListRequest(IdType packetId)
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

void ClientJsonRpc::handlePacket(int method, JsonRpc::PacketForm type,
                                 Connection *conn, const EndpointId replyTo,
                                 const Json::Value &root)
{
  switch (method) {
  case LIST_QUEUES:
  {
    switch (type) {
    default:
    case INVALID_PACKET:
    case NOTIFICATION_PACKET:
    case ERROR_PACKET:
    case REQUEST_PACKET:
      handleInvalidRequest(conn, replyTo, root);
      break;
    case RESULT_PACKET:
      handleListQueuesResult(root);
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
    case REQUEST_PACKET:
      handleInvalidRequest(conn, replyTo, root);
      break;
    case RESULT_PACKET:
      handleSubmitJobResult(root);
      break;
    case ERROR_PACKET:
      handleSubmitJobError(root);
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
    case REQUEST_PACKET:
      handleInvalidRequest(conn, replyTo, root);
      break;
    case RESULT_PACKET:
      handleCancelJobResult(root);
      break;
    case ERROR_PACKET:
      handleCancelJobError(root);
      break;
    }
    break;
  }
  case LOOKUP_JOB:
  {
    switch (type) {
    default:
    case INVALID_PACKET:
    case NOTIFICATION_PACKET:
    case REQUEST_PACKET:
      handleInvalidRequest(conn, replyTo, root);
      break;
    case RESULT_PACKET:
      handleLookupJobResult(root);
      break;
    case ERROR_PACKET:
      handleLookupJobError(root);
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
      handleInvalidRequest(conn, replyTo, root);
      break;
    case NOTIFICATION_PACKET:
      handleJobStateChangedNotification(root);
      break;
    }
    break;
  }
  default:
    handleInvalidRequest(conn, replyTo, root);
    break;
  }
}

void ClientJsonRpc::handleListQueuesResult(const Json::Value &root) const
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


  emit queueListReceived(id, queueList);
}

void ClientJsonRpc::handleSubmitJobResult(const Json::Value &root) const
{
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

  const Json::Value &resultObject = root["result"];

  IdType moleQueueId;
  QDir workingDirectory;

  if (!resultObject["moleQueueId"].isIntegral() ||
      !resultObject["workingDirectory"].isString()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job submission result is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  moleQueueId = static_cast<IdType>(
        resultObject["moleQueueId"].asLargestUInt());
  workingDirectory = QDir(QString(
                            resultObject["workingDirectory"].asCString()));

  emit successfulSubmissionReceived(id, moleQueueId, workingDirectory);
}

void ClientJsonRpc::handleSubmitJobError(const Json::Value &root) const
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

  const ErrorCode errorCode = static_cast<ErrorCode>(
        root["error"]["code"].asLargestInt());

  const QString errorMessage(root["error"]["message"].asCString());

  emit failedSubmissionReceived(id, errorCode, errorMessage);
}

void ClientJsonRpc::handleCancelJobResult(const Json::Value &root) const
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

void ClientJsonRpc::handleCancelJobError(const Json::Value &root) const
{
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

  if (!root["error"]["code"].isIntegral() ||
      !root["error"]["data"].isIntegral() ||
      !root["error"]["message"].isString()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job lookup failure response is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  const ErrorCode errorCode =
      static_cast<ErrorCode>(
        root["error"]["code"].asLargestUInt());
  const QString message(root["error"]["message"].asCString());
  const IdType moleQueueId =
      static_cast<IdType>(root["error"]["data"].asLargestUInt());

  emit jobCancellationErrorReceived(id, moleQueueId, errorCode, message);
}

void ClientJsonRpc::handleLookupJobResult(const Json::Value &root) const
{
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

  const Json::Value &resultObject = root["result"];

  if (!resultObject.isObject()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job lookup result is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  QVariantHash hash = QtJson::toVariant(resultObject).toHash();

  emit lookupJobResponseReceived(id, hash);
}

void ClientJsonRpc::handleLookupJobError(const Json::Value &root) const
{
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

  if (!root["error"]["code"].isIntegral() ||
      !root["error"]["data"].isIntegral() ||
      !root["error"]["message"].isString()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job lookup failure response is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  const IdType moleQueueId =
      static_cast<IdType>(root["error"]["data"].asLargestUInt());

  emit lookupJobErrorReceived(id, moleQueueId);
}

void ClientJsonRpc::handleJobStateChangedNotification(
    const Json::Value &root) const
{
  const Json::Value &paramsObject = root["params"];

  IdType moleQueueId;
  JobState oldState;
  JobState newState;

  if (!paramsObject.isObject() ||
      !paramsObject["moleQueueId"].isIntegral() ||
      !paramsObject["oldState"].isString() ||
      !paramsObject["newState"].isString() ){
    Json::StyledWriter writer;
    const std::string requestString = writer.write(root);
    qWarning() << "Job cancellation result is ill-formed:\n"
               << requestString.c_str();
    return;
  }

  moleQueueId = static_cast<IdType>(
        paramsObject["moleQueueId"].asLargestUInt());
  oldState = stringToJobState(paramsObject["oldState"].asCString());
  newState = stringToJobState(paramsObject["newState"].asCString());

  emit jobStateChangeReceived(moleQueueId, oldState, newState);
}

} // namespace MoleQueue
