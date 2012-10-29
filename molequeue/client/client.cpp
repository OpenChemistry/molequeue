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

#include "client.h"

#include "job.h"

#include <qjsondocument.h>
#include <QtCore/QDebug>
#include <QtCore/QDataStream>
#include <QtNetwork/QLocalSocket>

namespace MoleQueue
{

Client::Client(QObject *parent_) : QObject(parent_), m_socket(NULL)
{
  m_packetCounter = 0;
}

Client::~Client()
{
}

bool Client::isConnected() const
{
  if (!m_socket)
    return false;
  else
    return m_socket->isOpen();
}

bool Client::connectToServer(const QString &serverName)
{
  if (m_socket && m_socket->isOpen()) {
    if (m_socket->serverName() == serverName) {
      return false;
    }
    else {
      m_socket->close();
      delete m_socket;
      m_socket = NULL;
    }
  }

  // New connection.
  if (m_socket == NULL) {
    if (serverName.isEmpty()) {
      return false;
    }
    else {
      m_socket = new QLocalSocket(this);
      m_socket->connectToServer(serverName);
      connect(m_socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
      return true;
    }
  }
  return false;
}

int Client::requestQueueList()
{
  QJsonObject packet;
  emptyRequest(packet);

  packet["method"] = QLatin1String("listQueues");

  sendRequest(packet);
  int localId = static_cast<int>(packet["id"].toDouble());
  m_requests[localId] = ListQueues;
  return localId;
}

int Client::submitJob(const JobObject &job)
{
  QJsonObject packet;
  emptyRequest(packet);

  packet["method"] = QLatin1String("submitJob");
  packet["params"] = job.json();

  sendRequest(packet);
  int localId = static_cast<int>(packet["id"].toDouble());
  m_requests[localId] = SubmitJob;
  return localId;
}

int Client::lookupJob(unsigned int moleQueueId)
{
  QJsonObject packet;
  emptyRequest(packet);

  packet["method"] = QLatin1String("lookupJob");
  QJsonObject params;
  params["moleQueueId"] = static_cast<int>(moleQueueId);
  packet["params"] = params;
  sendRequest(packet);
  int localId = static_cast<int>(packet["id"].toDouble());
  m_requests[localId] = LookupJob;
  return localId;
}

int Client::cancelJob(unsigned int moleQueueId)
{
  QJsonObject packet;
  emptyRequest(packet);

  packet["method"] = QLatin1String("cancelJob");
  QJsonObject params;
  params["moleQueueId"] = static_cast<int>(moleQueueId);
  packet["params"] = params;
  sendRequest(packet);
  int localId = static_cast<int>(packet["id"].toDouble());
  m_requests[localId] = CancelJob;
  return localId;
}

void Client::flush()
{
  if (m_socket)
    m_socket->flush();
}

void Client::readPacket(const QByteArray message)
{
  // Read packet into a Json value
  QJsonParseError error;
  QJsonDocument reader = QJsonDocument::fromJson(message, &error);

  if (error.error != QJsonParseError::NoError) {
    qDebug() << "Unparseable message received\n:" << error.errorString()
             << "\nContent: " << message;
    return;
  }
  else if (!reader.isObject()) {
    // We need a valid object, something bad happened.
    return;
  }
  else {
    QJsonObject root = reader.object();
    if (root["method"] != QJsonValue::Null) {
      if (root["id"] != QJsonValue::Null)
        qDebug() << "Received a request packet - that shouldn't happen here.";
      else
        processNotification(root);
    }
    if (root["result"] != QJsonValue::Null) {
      // This is a result packet, and should emit a signal.
      processResult(root);
    }
    else if (root["error"] != QJsonValue::Null) {
      // FIXME: Add error signal here.
      qDebug() << "Error packet:\n" << message;
    }
  }

}

void Client::readSocket()
{
  QDataStream stream(m_socket);

  while (m_socket->bytesAvailable()) {
    QByteArray json;
    stream >> json;
    readPacket(json);
  }
}

void Client::emptyRequest(QJsonObject &request)
{
  request["jsonrpc"] = QLatin1String("2.0");
  request["id"] = static_cast<int>(m_packetCounter++);
}

void Client::sendRequest(const QJsonObject &request)
{
  if (!m_socket)
    return;

  QJsonDocument document(request);
  QDataStream stream(m_socket);
  stream.setVersion(QDataStream::Qt_4_8);
  stream << document.toJson();

  //m_connection->send(PacketType(jsonString.c_str()));
  qDebug() << "sending request:" << document.toJson();
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
    qDebug() << "We couldn't find a valid ID for the response :-(";
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

} // End namespace MoleQueue
