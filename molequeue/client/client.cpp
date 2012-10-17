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

void Client::requestQueueList()
{
  QJsonObject packet;
  emptyRequest(packet);

  packet["method"] = QLatin1String("listQueues");

  sendRequest(packet);
  m_requests[static_cast<int>(packet["id"].toDouble())] = QueueList;
}

void Client::submitJob(const JobObject &job)
{
  QJsonObject packet;
  emptyRequest(packet);

  packet["method"] = QLatin1String("submitJob");
  packet["params"] = job.json();

  sendRequest(packet);
  m_requests[static_cast<int>(packet["id"].toDouble())] = Submission;
}

void Client::lookupJob(unsigned int moleQueueId)
{
  QJsonObject packet;
  emptyRequest(packet);

  packet["method"] = QLatin1String("lookupJob");
  QJsonObject params;
  params["moleQueueId"] = static_cast<int>(moleQueueId);
  packet["params"] = params;
  sendRequest(packet);
  m_requests[static_cast<int>(packet["id"].toDouble())] = JobInfo;
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
        qDebug() << "Received a notification packet:\n" << message;
    }
    if (root["result"] != QJsonValue::Null) {
      // This is a result packet, and should emit a signal.
      if (root["id"] != QJsonValue::Null
          && m_requests.contains(static_cast<int>(root["id"].toDouble()))) {
        switch (m_requests[static_cast<int>(root["id"].toDouble())]) {
        case Submission:
          emit submitJobResponse(root["result"].toObject()["moleQueueId"].toDouble());
          break;
        case JobInfo:
          break;
        case QueueList:
          //emit queueListReceived();
          break;
        default:
          break;
        }
      }
      else {
        qDebug() << "We couldn't find a valid ID for the response :-(";
      }
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
  stream << document.toJson();

  //m_connection->send(PacketType(jsonString.c_str()));
  qDebug() << "sending request:" << document.toJson();
}

} // End namespace MoleQueue
