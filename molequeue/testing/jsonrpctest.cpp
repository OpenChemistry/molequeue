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

#include "transport/jsonrpc.h"
#include "dummyjsonrpc.h"

#include "transport/message.h"

#include <json/json.h>

#include <QtTest>
#include <QtCore/QFile>

using namespace MoleQueue;

class JsonRpcTest : public QObject
{
  Q_OBJECT

private:
  PacketType readReferenceString(const QString &filename);
  void printNode(const Json::Value &);

  bool m_error;
  DummyJsonRpc m_rpc;
  PacketType m_packet;
  Json::Reader m_reader;
  Json::StyledWriter m_writer;
  Json::Value m_root;

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  // Unit test functions go below:
  void validateRequest();
  void validateResponse();
  void validateNotification();

  void interpretIncomingMessage_unparsable();
  void interpretIncomingMessage_invalidRequest();
  void interpretIncomingMessage_unrecognizedRequest();
};

PacketType JsonRpcTest::readReferenceString(const QString &filename)
{
  QString realFilename = MoleQueue_TESTDATA_DIR + filename;
  QFile refFile (realFilename);
  if (!refFile.open(QFile::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot access reference file" << realFilename;
    return "";
  }
  PacketType contents = refFile.readAll();
  refFile.close();
  return contents;
}

void JsonRpcTest::printNode(const Json::Value &root)
{
  std::string str;
  str = m_writer.write(root);
  qDebug() << str.c_str();
}

void JsonRpcTest::initTestCase()
{
  qRegisterMetaType<MoleQueue::Connection*>("MoleQueue::Connection*");
  qRegisterMetaType<MoleQueue::EndpointIdType>("MoleQueue::EndpointIdType");
}

void JsonRpcTest::cleanupTestCase()
{

}

void JsonRpcTest::init()
{
  // Reset error monitor
  m_error = false;
}

void JsonRpcTest::cleanup()
{

}

void JsonRpcTest::validateRequest()
{
  // Test valid requests
  m_packet = readReferenceString("jsonrpc-ref/valid-requests.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading valid requests file (not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (!m_rpc.validateRequest(Message(*it), false)) {
        qDebug() << "Valid request failed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test invalid requests
  m_packet = readReferenceString("jsonrpc-ref/invalid-requests.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading invalid requests file (not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateRequest(Message(*it), false)) {
        qDebug() << "Invalid request passed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test strictly invalid requests
  m_packet = readReferenceString("jsonrpc-ref/strictly-invalid-requests.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading strictly invalid requests file "
                "(not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateRequest(Message(*it), true)) {
        qDebug() << "Strictly invalid request passed strict validation:";
        printNode(*it);
        m_error = true;
      }
      if (!m_rpc.validateRequest(Message(*it), false)) {
        qDebug() << "Strictly invalid request failed loose validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::validateResponse()
{
  // Test valid responses
  m_packet = readReferenceString("jsonrpc-ref/valid-responses.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading valid responses file (not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (!m_rpc.validateResponse(Message(*it), false)) {
        qDebug() << "Valid response failed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test invalid responses
  m_packet = readReferenceString("jsonrpc-ref/invalid-responses.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading invalid responses file (not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateResponse(Message(*it), false)) {
        qDebug() << "Invalid response passed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test strictly invalid responses
  m_packet = readReferenceString("jsonrpc-ref/strictly-invalid-responses.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading strictly invalid responses file "
                "(not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateResponse(Message(*it), true)) {
        qDebug() << "Strictly invalid response passed strict validation:";
        printNode(*it);
        m_error = true;
      }
      if (!m_rpc.validateResponse(Message(*it), false)) {
        qDebug() << "Strictly invalid response failed loose validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::validateNotification()
{
  // Test valid notifications
  m_packet = readReferenceString("jsonrpc-ref/valid-notifications.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading valid notifications file (not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (!m_rpc.validateNotification(Message(*it), false)) {
        qDebug() << "Valid notification failed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test invalid notifications
  m_packet = readReferenceString("jsonrpc-ref/invalid-notifications.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading invalid notifications file "
                "(not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateNotification(Message(*it), false)) {
        qDebug() << "Invalid notification passed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test strictly invalid notifications
  m_packet = readReferenceString("jsonrpc-ref/strictly-invalid-notifications.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading strictly invalid notifications file "
                "(not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateNotification(Message(*it), true)) {
        qDebug() << "Strictly invalid notification passed strict validation:";
        printNode(*it);
        m_error = true;
      }
      if (!m_rpc.validateNotification(Message(*it), false)) {
        qDebug() << "Strictly invalid notification failed loose validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::interpretIncomingMessage_unparsable()
{
  QSignalSpy spy(&m_rpc, SIGNAL(invalidMessageReceived(MoleQueue::Message,
                                                       Json::Value)));
  m_packet = "[I'm not valid JSON!}";
  m_rpc.interpretIncomingMessage(Message(m_packet));

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingMessage_invalidRequest()
{
  QSignalSpy spy(&m_rpc, SIGNAL(invalidRequestReceived(MoleQueue::Message,
                                                       Json::Value)));
  m_packet = "{\"I'm valid JSON\" : \"but not JSON-RPC\"}";
  m_rpc.interpretIncomingMessage(Message(m_packet));

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingMessage_unrecognizedRequest()
{
  QSignalSpy spy(&m_rpc,
                 SIGNAL(unrecognizedRequestReceived(MoleQueue::Message,
                                                    Json::Value)));
  m_packet = "{ \"jsonrpc\" : \"2.0\", \"id\" : \"0\", \"method\" : \"asdf\" }";
  m_rpc.interpretIncomingMessage(Message(m_packet));

  QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(JsonRpcTest)

#include "jsonrpctest.moc"
