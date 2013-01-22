/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2012-2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "client.h"

#include "jsonrpcclient.h"
#include "job.h"

#include <qjsondocument.h>

namespace MoleQueue
{

Client::Client(QObject *parent_) : QObject(parent_), m_jsonRpcClient(NULL)
{
}

Client::~Client()
{
}

bool Client::isConnected() const
{
  if (!m_jsonRpcClient)
    return false;
  else
    return m_jsonRpcClient->isConnected();
}

bool Client::connectToServer(const QString &serverName)
{
  if (!m_jsonRpcClient) {
    m_jsonRpcClient = new JsonRpcClient(this);
    connect(m_jsonRpcClient, SIGNAL(resultReceived(QJsonObject)),
            SLOT(processResult(QJsonObject)));
    connect(m_jsonRpcClient, SIGNAL(notificationReceived(QJsonObject)),
            SLOT(processNotification(QJsonObject)));
    connect(m_jsonRpcClient, SIGNAL(errorReceived(QJsonObject)),
            SLOT(processError(QJsonObject)));
    connect(m_jsonRpcClient, SIGNAL(connectionStateChanged()),
            SIGNAL(connectionStateChanged()));
  }

  return m_jsonRpcClient->connectToServer(serverName);
}

int Client::requestQueueList()
{
  if (!m_jsonRpcClient)
    return -1;

  QJsonObject packet = m_jsonRpcClient->emptyRequest();
  packet["method"] = QLatin1String("listQueues");
  if (!m_jsonRpcClient->sendRequest(packet))
    return -1;

  int localId = static_cast<int>(packet["id"].toDouble());
  m_requests[localId] = ListQueues;
  return localId;
}

int Client::submitJob(const JobObject &job)
{
  if (!m_jsonRpcClient)
    return -1;

  QJsonObject packet = m_jsonRpcClient->emptyRequest();
  packet["method"] = QLatin1String("submitJob");
  packet["params"] = job.json();
  if (!m_jsonRpcClient->sendRequest(packet))
    return -1;

  int localId = static_cast<int>(packet["id"].toDouble());
  m_requests[localId] = SubmitJob;
  return localId;
}

int Client::lookupJob(unsigned int moleQueueId)
{
  if (!m_jsonRpcClient)
    return -1;

  QJsonObject packet = m_jsonRpcClient->emptyRequest();
  packet["method"] = QLatin1String("lookupJob");
  QJsonObject params;
  params["moleQueueId"] = static_cast<int>(moleQueueId);
  packet["params"] = params;
  if (!m_jsonRpcClient->sendRequest(packet))
    return -1;

  int localId = static_cast<int>(packet["id"].toDouble());
  m_requests[localId] = LookupJob;
  return localId;
}

int Client::cancelJob(unsigned int moleQueueId)
{
  if (!m_jsonRpcClient)
    return -1;

  QJsonObject packet = m_jsonRpcClient->emptyRequest();
  packet["method"] = QLatin1String("cancelJob");
  QJsonObject params;
  params["moleQueueId"] = static_cast<int>(moleQueueId);
  packet["params"] = params;
  if (!m_jsonRpcClient->sendRequest(packet))
    return -1;

  int localId = static_cast<int>(packet["id"].toDouble());
  m_requests[localId] = CancelJob;
  return localId;
}

void Client::flush()
{
  if (m_jsonRpcClient)
    m_jsonRpcClient->flush();
}

void Client::processResult(const QJsonObject &response)
{
  if (response["id"] != QJsonValue::Null
      && m_requests.contains(static_cast<int>(response["id"].toDouble()))) {
    int localId = static_cast<int>(response["id"].toDouble());
    switch (m_requests[localId]) {
    case ListQueues:
      emit queueListReceived(response["result"].toObject());
      break;
    case SubmitJob:
      emit submitJobResponse(localId,
                             static_cast<unsigned int>(response["result"]
                             .toObject()["moleQueueId"].toDouble()));
      break;
    case LookupJob:
      emit lookupJobResponse(localId, response["result"].toObject());
      break;
    case CancelJob:
      emit cancelJobResponse(static_cast<unsigned int>(response["result"]
                             .toObject()["moleQueueId"].toDouble()));
    default:
      break;
    }
  }
  else {
    emit errorReceived("We couldn't find a valid ID for the response.");
  }
}

void Client::processNotification(const QJsonObject &notification)
{
  if (notification["method"].toString() == "jobStateChanged") {
    QJsonObject params = notification["params"].toObject();
    emit jobStateChanged(
          static_cast<unsigned int>(params["moleQueueId"].toDouble()),
          params["oldState"].toString(), params["newState"].toString());
  }
}

void Client::processError(const QJsonObject &error)
{
  emit errorReceived(static_cast<int>(error["id"].toDouble()),
                     static_cast<unsigned int>(0),
                     error["error"].toObject()["message"].toString());
}

} // End namespace MoleQueue
