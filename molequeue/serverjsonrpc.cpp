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

PacketType ServerJsonRpc::generateJobSubmissionConfirmation(
    IdType moleQueueId, const QString &workingDir, IdType packetId)
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
    IdType moleQueueId, IdType packetId)
{
  Json::Value packet = generateEmptyResponse(packetId);

  packet["result"] = moleQueueId;

  Json::StyledWriter writer;
  std::string ret_stdstr = writer.write(packet);
  PacketType ret(ret_stdstr.c_str());

  return ret;
}

PacketType ServerJsonRpc::generateLookupJobResponse(
    const Job &req, IdType moleQueueId, IdType packetId)
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
                                            IdType packetId)
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

void ServerJsonRpc::handlePacket(int method, JsonRpc::PacketForm type,
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
    case RESULT_PACKET:
    case ERROR_PACKET:
      handleInvalidRequest(conn, replyTo, root);
      break;
    case REQUEST_PACKET:
      handleListQueuesRequest(conn, replyTo, root);
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
    case RESULT_PACKET:
    case ERROR_PACKET:
      handleInvalidRequest(conn, replyTo, root);
      break;
    case REQUEST_PACKET:
      handleSubmitJobRequest(conn, replyTo, root);
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
    case RESULT_PACKET:
    case ERROR_PACKET:
      handleInvalidRequest(conn, replyTo, root);
      break;
    case REQUEST_PACKET:
      handleCancelJobRequest(conn, replyTo, root);
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
    case RESULT_PACKET:
    case ERROR_PACKET:
      handleInvalidRequest(conn, replyTo, root);
      break;
    case REQUEST_PACKET:
      handleLookupJobRequest(conn, replyTo, root);
      break;
    }
    break;
  }
  case JOB_STATE_CHANGED:
  {
    handleInvalidRequest(conn, replyTo, root);
    break;
  }
  default:
    handleInvalidRequest(conn, replyTo, root);
    break;
  }

}

void ServerJsonRpc::handleListQueuesRequest(Connection *connection,
                                            const EndpointId replyTo,
                                            const Json::Value &root) const
{
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());
  emit queueListRequestReceived(connection, replyTo, id);
}

void ServerJsonRpc::handleSubmitJobRequest(Connection *connection,
                                           const EndpointId replyTo,
                                           const Json::Value &root) const
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
  QVariantHash optionHash = QtJson::toVariant(paramsObject).toHash();

  emit jobSubmissionRequestReceived(connection, replyTo, id, optionHash);
}

void ServerJsonRpc::handleCancelJobRequest(Connection *connection,
                                           const EndpointId replyTo,
                                           const Json::Value &root) const
{
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

  const Json::Value &paramsObject = root["params"];

  if (!paramsObject.isObject() ||
      !paramsObject["moleQueueId"].isIntegral()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job cancellation request is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  const IdType moleQueueId = static_cast<IdType>(
        paramsObject["moleQueueId"].asLargestUInt());

  emit jobCancellationRequestReceived(connection, replyTo, id, moleQueueId);
}

void ServerJsonRpc::handleLookupJobRequest(Connection *connection,
                                           const EndpointId replyTo,
                                           const Json::Value &root) const
{
  const IdType id = static_cast<IdType>(root["id"].asLargestUInt());

  const Json::Value &paramsObject = root["params"];

  if (!paramsObject.isObject() ||
      !paramsObject["moleQueueId"].isIntegral()) {
    Json::StyledWriter writer;
    const std::string responseString = writer.write(root);
    qWarning() << "Job lookup request is ill-formed:\n"
               << responseString.c_str();
    return;
  }

  const IdType moleQueueId = static_cast<IdType>(
        paramsObject["moleQueueId"].asLargestUInt());

  emit lookupJobRequestReceived(connection, replyTo, id, moleQueueId);
}

} // namespace MoleQueue
