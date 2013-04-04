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

#include "jsonrpcclient.h"

#include <qjsondocument.h>
#include <QtCore/QDataStream>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtNetwork/QLocalSocket>

namespace {
/** Helper for the static pingServer method. */
class PingListener : public QObject
{
  Q_OBJECT
public:
  PingListener() : replyReceived(false), pingSuccessful(false) {}
  virtual ~PingListener() {}
  bool replyReceived;
  bool pingSuccessful;
public slots:
  void receivePong(const QJsonObject &response)
  {
    pingSuccessful = (response.value("result").toString() == QString("pong"));
    replyReceived = true;
  }
};
}

namespace MoleQueue
{

JsonRpcClient::JsonRpcClient(QObject *parent_) :
  QObject(parent_),
  m_packetCounter(0),
  m_socket(NULL)
{
}

JsonRpcClient::~JsonRpcClient()
{
}

bool JsonRpcClient::isConnected() const
{
  if (!m_socket)
    return false;
  else
    return m_socket->isOpen();
}

bool JsonRpcClient::connectToServer(const QString &serverName_)
{
  if (m_socket && m_socket->isOpen()) {
    if (m_socket->serverName() == serverName_) {
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
    m_socket = new QLocalSocket(this);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
  }

  if (serverName_.isEmpty()) {
    return false;
  }
  else {
    m_socket->connectToServer(serverName_);
    return isConnected();
  }
}

QString JsonRpcClient::serverName() const
{
  if (m_socket)
    return m_socket->serverName();
  else
    return QString();
}

bool JsonRpcClient::pingServer(const QString &serverName, int msTimeout)
{
  MoleQueue::JsonRpcClient client;
  PingListener listener;
  QTimer timeout;
  timeout.setInterval(msTimeout);
  timeout.setSingleShot(true);

  bool result(client.connectToServer(serverName));

  if (result) {
    QJsonObject request(client.emptyRequest());
    request["method"] = QLatin1String("internalPing");
    QObject::connect(&client, SIGNAL(resultReceived(QJsonObject)),
                     &listener, SLOT(receivePong(QJsonObject)));
    result = client.sendRequest(request);
    timeout.start();
  }

  // Wait for request
  if (result) {
    while (!listener.replyReceived && timeout.isActive())
      qApp->processEvents();
    result = listener.pingSuccessful;
  }

  return result;
}

void JsonRpcClient::flush()
{
  if (m_socket)
    m_socket->flush();
}

QJsonObject JsonRpcClient::emptyRequest()
{
  QJsonObject request;
  request["jsonrpc"] = QLatin1String("2.0");
  request["id"] = static_cast<int>(m_packetCounter++);
  return request;
}

bool JsonRpcClient::sendRequest(const QJsonObject &request)
{
  if (!m_socket)
    return false;

  QJsonDocument document(request);
  QDataStream stream(m_socket);
  stream.setVersion(QDataStream::Qt_4_8);
  stream << document.toJson();
  return true;
}

void JsonRpcClient::readPacket(const QByteArray message)
{
  // Read packet into a Json value
  QJsonParseError error;
  QJsonDocument reader = QJsonDocument::fromJson(message, &error);

  if (error.error != QJsonParseError::NoError) {
    emit badPacketReceived("Unparseable message received\n:"
                           + error.errorString() + "\nContent: " + message);
    return;
  }
  else if (!reader.isObject()) {
    // We need a valid object, something bad happened.
    emit badPacketReceived("Packet did not contain a valid JSON object.");
    return;
  }
  else {
    QJsonObject root = reader.object();
    if (root["method"] != QJsonValue::Null) {
      if (root["id"] != QJsonValue::Null)
        emit badPacketReceived("Received a request packet for the client.");
      else
        emit notificationReceived(root);
    }
    if (root["result"] != QJsonValue::Null) {
      // This is a result packet, and should emit a signal.
      emit resultReceived(root);
    }
    else if (root["error"] != QJsonValue::Null) {
      emit errorReceived(root);
    }
  }
}

void JsonRpcClient::readSocket()
{
  QDataStream stream(m_socket);

  while (m_socket->bytesAvailable()) {
    QByteArray json;
    stream >> json;
    readPacket(json);
  }
}

} // End namespace MoleQueue

// For the moc'd PingListener
#include "jsonrpcclient.moc"
