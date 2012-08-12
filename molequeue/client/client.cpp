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

#include <json/json.h>

#include <molequeue/transport/message.h>
#include <molequeue/transport/localsocket/localsocketconnection.h>

#include <QtCore/QDebug>
#include <QtCore/QDataStream>
#include <QtNetwork/QLocalSocket>

namespace MoleQueue
{

Client::Client(QObject *parent_) : QObject(parent_), m_socket(NULL)
{
  // Randomize the packet counter's starting value.
  qsrand(static_cast<uint>(QDateTime::currentMSecsSinceEpoch()));
  m_packetCounter = static_cast<unsigned int>(qrand());
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

void Client::connectToServer(const QString &serverName)
{
  if (m_socket && m_socket->isOpen()) {
    if (m_socket->serverName() == serverName) {
      return;
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
      qDebug() << "Server name is empty...";
      return;
    }
    else {
      qDebug() << "Opening connection to" << serverName;
      m_socket = new QLocalSocket(this);
      m_socket->connectToServer(serverName);
      connect(m_socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    }
  }
}

void Client::requestQueueList()
{
  Json::Value packet;
  emptyRequest(packet);

  packet["method"] = "listQueues";

  sendRequest(packet);
  m_requests[packet["id"].asInt()] = QueueList;
}

void Client::submitJob(const JobObject &job)
{
  Json::Value packet;
  emptyRequest(packet);

  packet["method"] = "submitJob";
  packet["params"] = job.json();

  sendRequest(packet);
  m_requests[packet["id"].asInt()] = Submission;
}

void Client::lookupJob(unsigned int moleQueueId)
{
  Json::Value packet;
  emptyRequest(packet);

  packet["method"] = "lookupJob";
  Json::Value params;
  params["moleQueueId"] = moleQueueId;
  packet["params"] = params;
  sendRequest(packet);
  m_requests[packet["id"].asInt()] = JobInfo;
}

void Client::readPacket(const QByteArray message)
{
  //qDebug() << "Packet received:" << message.data();

  // Read packet into a Json value
  Json::Reader reader;
  Json::Value root;

  if (!reader.parse(message.constData(),
                    message.constData() + message.size(),
                    root, false)) {
    qDebug() << "Unparseable message received\n:" << message;
    return;
  }
  else {
    qDebug() << "JSON interpreted successfully:" << endl;
    if (!root.isObject())
      qDebug() << "Invalid packet received:\n" << message;

    if (root["method"] != Json::nullValue) {
      if (root["id"] != Json::nullValue)
        qDebug() << "Received a request packet - that shouldn't happen here.";
      else
        qDebug() << "Received a notification packet:\n" << message;
    }
    if (root["result"] != Json::nullValue) {
      qDebug() << "Result packet - that's more like it!\n" << message;
      if (root["id"] != Json::nullValue && m_requests.contains(root["id"].asInt())) {

        switch (m_requests[root["id"].asInt()]) {
        case Submission:
          emit submitJobResponse(root["result"]["moleQueueId"].asInt());
          break;
        case JobInfo:
          break;
        case QueueList:
          //emit queueListReceived();
          break;
        }

        qDebug() << "We have a valid result packet!!!"
                 << "id:" << root["id"].asInt() << "type" << m_requests[root["id"].asInt()];
      }
      else {
        qDebug() << "We couldn't find a valid ID for the response :-(";
      }
    }
    else if (root["error"] != Json::nullValue)
      qDebug() << "Error packet:\n" << message;
  }

}

void Client::readSocket()
{
  QDataStream stream(m_socket);
  qDebug() << "readSocket on client triggered - checking what we have...";

  while (m_socket->bytesAvailable()) {
    QByteArray json;
    stream >> json;
    //qDebug() << "JSON received:" << json;
    readPacket(json);
  }
}

void Client::emptyRequest(Json::Value &request)
{
  request["jsonrpc"] = "2.0";
  request["id"] = m_packetCounter++;
}

void Client::sendRequest(const Json::Value &request)
{
  if (!m_socket)
    return;

  Json::StyledWriter writer;
  std::string jsonString = writer.write(request);

  QDataStream stream(m_socket);
  stream << QByteArray(jsonString.c_str());

  //m_connection->send(PacketType(jsonString.c_str()));
  qDebug() << "sending request:" << jsonString.c_str();
}

} // End namespace MoleQueue
