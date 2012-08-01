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

#include <QtTest>

#include "abstractrpcinterface.h"
#include "molequeueglobal.h"
#include "testserver.h"
#include "transport/localsocket/localsocketconnection.h"

#include <QtNetwork/QLocalSocket>

class AbstractRpcInterfaceTest : public QObject
{
  Q_OBJECT

private:
  TestServer *m_server;
  MoleQueue::LocalSocketConnection *m_connection;
  MoleQueue::AbstractRpcInterface *m_rpc;
  MoleQueue::PacketType m_packet;

private slots:
  MoleQueue::PacketType readReferenceString(const QString &filename);

  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void testInvalidPacket();
  void testInvalidRequest();
  void testInvalidMethod();
  void testInvalidParams();
  void testInternalError();
};


MoleQueue::PacketType
AbstractRpcInterfaceTest::readReferenceString(const QString &filename)
{
  QString realFilename = TESTDATADIR + filename;
  QFile refFile (realFilename);
  if (!refFile.open(QFile::ReadOnly | QFile::Text)) {
    qDebug() << "Cannot access reference file" << realFilename;
    return "";
  }
  MoleQueue::PacketType contents = refFile.readAll();
  refFile.close();
  return contents;
}

void AbstractRpcInterfaceTest::initTestCase()
{
  m_server = new TestServer(&m_packet);
  m_rpc = new MoleQueue::AbstractRpcInterface();
  m_connection =
      new MoleQueue::LocalSocketConnection(this, m_server->socketName());
  m_connection->open();
  m_connection->start();

  connect(m_connection, SIGNAL(newMessage(const MoleQueue::Message)),
          m_rpc, SLOT(readPacket(const MoleQueue::Message)));


  // Let the event loop run a bit to handle the connections
  qApp->processEvents(QEventLoop::AllEvents, 1000);
}

void AbstractRpcInterfaceTest::cleanupTestCase()
{
  delete m_server;
  delete m_rpc;
  delete m_connection;
}

void AbstractRpcInterfaceTest::init()
{
  m_packet.clear();
}

void AbstractRpcInterfaceTest::cleanup()
{
}

void AbstractRpcInterfaceTest::testInvalidPacket()
{
  MoleQueue::PacketType packet =
      "{ 42 \"I'm malformed JSON! ]";
  m_server->sendPacket(packet);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      readReferenceString("abstractrpcinterface-ref/invalid-json.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void AbstractRpcInterfaceTest::testInvalidRequest()
{
  MoleQueue::PacketType packet = "[1]"; // not an object
  m_server->sendPacket(packet);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      readReferenceString("abstractrpcinterface-ref/invalid-request.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void AbstractRpcInterfaceTest::testInvalidMethod()
{
  MoleQueue::PacketType packet =
      "{ \"jsonrpc\" : \"2.0\", \"id\" : 0, \"method\" : \"notARealMethod\"}";
  m_server->sendPacket(packet);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      readReferenceString("abstractrpcinterface-ref/invalid-method.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void AbstractRpcInterfaceTest::testInvalidParams()
{
  qDebug() << "There is currently no way to trigger an invalid parameter response.";
}

void AbstractRpcInterfaceTest::testInternalError()
{
  qDebug() << "There is currently no way to trigger an internal error response.";
}

QTEST_MAIN(AbstractRpcInterfaceTest)

#include "moc_abstractrpcinterfacetest.cxx"
