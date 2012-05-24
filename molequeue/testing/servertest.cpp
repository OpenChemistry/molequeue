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

#include "server.h"

#include "molequeueglobal.h"
#include "serverconnection.h"

#include <QtGui/QApplication>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include <QtCore/QVariantList>

class ServerTest : public QObject
{
  Q_OBJECT

private:
  QLocalSocket m_testSocket;
  MoleQueue::Server m_server;

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void testStart();
  void testStop();
  void testForceStart();

  void testNewConnection();
  void testClientDisconnected();
};

void ServerTest::initTestCase()
{
  m_server.m_isTesting = true;
  m_server.setDebug(true);
}

void ServerTest::cleanupTestCase()
{
}

void ServerTest::init()
{
}

void ServerTest::cleanup()
{
}

void ServerTest::testStart()
{
  m_server.start();
  QCOMPARE(m_server.m_server->serverName(), QString("MoleQueue-testing"));
}

void ServerTest::testStop()
{
  m_server.stop();
  QVERIFY(!m_server.m_server->isListening());
}

void ServerTest::testForceStart()
{
  // Start a duplicate server to take the socket address
  MoleQueue::Server dupServer;
  dupServer.m_isTesting = true;
  dupServer.setDebug(true);
  dupServer.start();

  // Attempt to start the server. Check that the AddressInUseError is emitted.
  QSignalSpy spy (&m_server,
                  SIGNAL(connectionError(QAbstractSocket::SocketError,QString)));
  m_server.start();
  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 2);
  QAbstractSocket::SocketError err =
      spy.first()[0].value<QAbstractSocket::SocketError>();
  QString errString = spy.first()[1].toString();
  QCOMPARE(err, QAbstractSocket::AddressInUseError);
  QCOMPARE(errString, QString("QLocalServer::listen: Address in use"));
  spy.clear();

  // Force start server
  m_server.forceStart();
  QVERIFY(spy.isEmpty());

  // Check that m_server is now listening.
  QVERIFY(m_server.m_server->isListening());

  dupServer.stop();
}

void ServerTest::testNewConnection()
{
  // Restart server to reset state
  m_server.stop();
  m_server.start();

  QSignalSpy spy (&m_server, SIGNAL(newConnection(ServerConnection*)));

  m_testSocket.connectToServer("MoleQueue-testing");
  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QVERIFY(m_testSocket.state() == QLocalSocket::ConnectedState);

  // Check that we've received the connection
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 1);
}

void ServerTest::testClientDisconnected()
{
  QCOMPARE(m_server.m_connections->size(), 1);
  m_testSocket.disconnectFromServer();
  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(m_server.m_connections->size(), 0);
}

QTEST_MAIN(ServerTest)

#include "moc_servertest.cxx"
