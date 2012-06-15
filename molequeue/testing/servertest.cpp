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
#include "connectionlistener.h"
#include "transport/localsocketconnectionlistener.h"

#include <QtGui/QApplication>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include <QtCore/QVariantList>

#include <assert.h>

class ServerTest : public QObject
{
  Q_OBJECT

private:
  QLocalSocket m_testSocket;
  MoleQueue::Server m_server;

  MoleQueue::LocalSocketConnectionListener * localSocketConnectionListener();

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

MoleQueue::LocalSocketConnectionListener * ServerTest::localSocketConnectionListener()
{

  MoleQueue::LocalSocketConnectionListener *localListener =
      static_cast<MoleQueue::LocalSocketConnectionListener *>(
          m_server.m_connectionListener);

   assert(localListener != NULL);

   return localListener;
}

void ServerTest::testStart()
{
  m_server.start();
  QCOMPARE(m_server.m_connectionListener->connectionString(),
           QString("MoleQueue-testing"));
}

void ServerTest::testStop()
{
  m_server.stop();

  QVERIFY(m_server.m_connectionListener == NULL);
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
                  SIGNAL(connectionError(MoleQueue::ConnectionListener::Error,const QString&)));
  m_server.start();
  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 2);
  MoleQueue::ConnectionListener::Error err =
      spy.first()[0].value<MoleQueue::ConnectionListener::Error>();
  QString errString = spy.first()[1].toString();
  QCOMPARE(err, MoleQueue::ConnectionListener::AddressInUseError); // TODO work out how to make this
  QCOMPARE(errString, QString("QLocalServer::listen: Address in use"));
  spy.clear();

  // Force start server
  m_server.forceStart();
  QVERIFY(spy.isEmpty());

  // Check that m_server is now listening.
  QVERIFY(this->localSocketConnectionListener()->m_server->isListening());

  dupServer.stop();
}

void ServerTest::testNewConnection()
{
  // Restart server to reset state
  m_server.stop();
  m_server.start();

  QSignalSpy spy (&m_server, SIGNAL(newConnection(MoleQueue::ServerConnection*)));

  m_testSocket.connectToServer("MoleQueue-testing");
  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QVERIFY(m_testSocket.state() == QLocalSocket::ConnectedState);

  // Check that we've received the connection
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 1);
}

void ServerTest::testClientDisconnected()
{
  QCOMPARE(m_server.m_connections.size(), 1);
  m_testSocket.disconnectFromServer();
  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(m_server.m_connections.size(), 0);
}

QTEST_MAIN(ServerTest)

#include "moc_servertest.cxx"
