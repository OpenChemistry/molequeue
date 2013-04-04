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
#include "transport/jsonrpc.h"

#include "dummyconnection.h"
#include "dummyconnectionlistener.h"
#include "referencestring.h"

#include "transport/message.h"

#include <qjsondocument.h>
#include <qjsonvalue.h>

#include <QtCore/QCoreApplication>

class JsonRpcTest : public QObject
{
  Q_OBJECT

private:
  DummyConnection m_conn1;
  DummyConnection *m_conn2;
  DummyConnectionListener m_connList1;
  DummyConnectionListener *m_connList2;
  MoleQueue::JsonRpc m_jsonRpc;

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void addConnectionListener();
  void addConnection();
  void messageReceived();
  void removeConnection();
  void removeConnectionListener();
  void internalPing();
};

void JsonRpcTest::initTestCase()
{
  m_conn2 = NULL;
  m_connList2 = NULL;
}

void JsonRpcTest::cleanupTestCase()
{
}

void JsonRpcTest::init()
{
}

void JsonRpcTest::cleanup()
{
}

void JsonRpcTest::addConnectionListener()
{
  QCOMPARE(m_jsonRpc.m_connections.size(), 0);
  m_jsonRpc.addConnectionListener(&m_connList1);
  QCOMPARE(m_jsonRpc.m_connections.size(), 1);
  m_connList2 = new DummyConnectionListener(this);
  m_jsonRpc.addConnectionListener(m_connList2);
  QCOMPARE(m_jsonRpc.m_connections.size(), 2);
}

void JsonRpcTest::addConnection()
{
  QCOMPARE(m_jsonRpc.m_connections.size(), 2);
  QCOMPARE(m_jsonRpc.m_connections[&m_connList1].size(), 0);
  m_connList1.emitNewConnection(&m_conn1);
  QCOMPARE(m_jsonRpc.m_connections[&m_connList1].size(), 1);
  m_conn2 = new DummyConnection(this);
  m_connList1.emitNewConnection(m_conn2);
  QCOMPARE(m_jsonRpc.m_connections[&m_connList1].size(), 2);
}

void JsonRpcTest::messageReceived()
{
  MoleQueue::Message dummyMsg(MoleQueue::Message::Request, &m_conn1);
  dummyMsg.setMethod("testMethod");
  QSignalSpy spy(&m_jsonRpc, SIGNAL(messageReceived(MoleQueue::Message)));
  m_conn1.emitPacketReceived(dummyMsg);
  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::removeConnection()
{
  // Destroying a connection should remove it from the JsonRpc instance. This
  // tests all code paths involved in removing a connection.
  QVERIFY(m_conn2 != NULL);
  QCOMPARE(m_jsonRpc.m_connections[&m_connList1].size(), 2);
  delete m_conn2;
  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(m_jsonRpc.m_connections[&m_connList1].size(), 1);
}

void JsonRpcTest::removeConnectionListener()
{
  // Destroying a connectionlistener should remove it from the JsonRpc instance.
  // This tests all code paths involved in removing a connectionlistener.
  QVERIFY(m_connList2 != NULL);
  QCOMPARE(m_jsonRpc.m_connections.size(), 2);
  delete m_connList2;
  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(m_jsonRpc.m_connections.size(), 1);
}

void JsonRpcTest::internalPing()
{
  ReferenceString request("jsonrpc-ref/internalPing-request.json");
  ReferenceString response("jsonrpc-ref/internalPing-response.json");
  DummyConnection connection;
  QJsonDocument doc(QJsonDocument::fromJson(request.toString().toLatin1()));
  m_jsonRpc.handleJsonValue(&connection, MoleQueue::EndpointIdType(),
                            doc.object());
  qApp->processEvents();
  QCOMPARE(QString(connection.popMessage().toJson()),
           QString(response.toString().toLatin1()));
}

QTEST_MAIN(JsonRpcTest)

#include "jsonrpctest.moc"
