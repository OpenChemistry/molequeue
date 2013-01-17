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

#include "molequeuetestconfig.h"

#include "jobmanager.h"
#include "molequeueglobal.h"
#include "program.h"
#include "transport/connectionlistener.h"
#include "transport/localsocket/localsocketconnectionlistener.h"
#include "transport/message.h"
#include "testing/dummyconnection.h"
#include "testing/referencestring.h"
#include "testing/testserver.h"
#include "queue.h"
#include "queuemanager.h"

#include <qjsondocument.h>

#include <QtGui/QApplication>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include <QtCore/QVariantList>

#include <assert.h>

using namespace MoleQueue;

class ServerTest : public QObject
{
  Q_OBJECT

private:
  QString m_connectionString;
  QLocalSocket m_testSocket;
  Server *m_server;

  LocalSocketConnectionListener * localSocketConnectionListener();

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

  void handleMessage_data();
  void handleMessage();
};

void ServerTest::initTestCase()
{
  // Change qsettings so that we don't overwrite the installed configuration:
  QString workDir = MoleQueue_BINARY_DIR "/Testing/Temporary/ServerTest";
  QDir dir;
  dir.mkpath(workDir);
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                     workDir + "/config");
  QSettings settings;
  settings.setValue("workingDirectoryBase", workDir);

  m_connectionString = TestServer::getRandomSocketName();
  m_server = new Server(this, m_connectionString);

  // Setup some fake queues/programs for rpc testing
  Queue *testQueue = m_server->queueManager()->addQueue("testQueue", "Local");
  Program *testProgram = new Program(testQueue);
  testProgram->setName("testProgram");
  testQueue->addProgram(testProgram);

  Queue *fakeQueue = m_server->queueManager()->addQueue("fakeQueue", "Local");
  Program *fakeProgram1 = new Program(fakeQueue);
  fakeProgram1->setName("fakeProgram1");
  fakeQueue->addProgram(fakeProgram1);
  Program *fakeProgram2 = new Program(fakeQueue);
  fakeProgram2->setName("fakeProgram2");
  fakeQueue->addProgram(fakeProgram2);
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

MoleQueue::LocalSocketConnectionListener *
  ServerTest::localSocketConnectionListener()
{

  MoleQueue::LocalSocketConnectionListener *localListener = NULL;

  foreach(MoleQueue::ConnectionListener *listener,
          m_server->m_connectionListeners) {
    localListener =
       static_cast<MoleQueue::LocalSocketConnectionListener *>(listener);

    if (localListener)
      break;
  }

   assert(localListener != NULL);

  return localListener;
}

void ServerTest::testStart()
{
  m_server->start();
  //QCOMPARE(m_server.m_connectionListener->connectionString(),
  //         QString("MoleQueue-testing"));
}

void ServerTest::testStop()
{
  m_server->stop(true);

  //QVERIFY(m_server.m_connectionListener == NULL);
}

void ServerTest::testForceStart()
{
// For now exclude this on Windows named pipes do now throw an error
// when you create one using the same name ...
#ifndef _WIN32

  // Start a duplicate server to take the socket address
  MoleQueue::Server dupServer(this, m_connectionString);
  dupServer.start();

  // Attempt to start the server. Check that the AddressInUseError is emitted.
  QSignalSpy spy (m_server,
                  SIGNAL(connectionError(MoleQueue::ConnectionListener::Error,
                                         const QString&)));
  m_server->start();
  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 2);
  MoleQueue::ConnectionListener::Error err =
      spy.first()[0].value<MoleQueue::ConnectionListener::Error>();
  QString errString = spy.first()[1].toString();
  QCOMPARE(err, MoleQueue::ConnectionListener::AddressInUseError);
  QCOMPARE(errString, QString("QLocalServer::listen: Address in use"));
  spy.clear();

  // Force start server
  m_server->forceStart();
  QVERIFY(spy.isEmpty());

  // Check that m_server is now listening.
  QVERIFY(localSocketConnectionListener()->m_server->isListening());

  dupServer.stop();
#endif

}

void ServerTest::testNewConnection()
{
  // Restart server to reset state
  m_server->stop();
  m_server->start();

  int origConns = m_server->m_connections.size();
  m_testSocket.connectToServer(m_connectionString);
  // Wait 5 seconds for a timeout.
  QTimer timer;
  timer.setSingleShot(true);
  timer.start(5000);
  while (timer.isActive() && m_server->m_connections.size() == origConns)
    qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(m_testSocket.state(), QLocalSocket::ConnectedState);

  // Check that we've received the connections
  // One zeromq and one local socket ...
#ifdef USE_ZERO_MQ
  QCOMPARE(m_server->m_connections.size(), 2);
#else
  QCOMPARE(m_server->m_connections.size(), 1);
#endif
}

void ServerTest::testClientDisconnected()
{
#ifdef USE_ZERO_MQ
  QCOMPARE(m_server->m_connections.size(), 2);
#else
  QCOMPARE(m_server->m_connections.size(), 1);
#endif

  int origConns = m_server->m_connections.size();
  m_testSocket.disconnectFromServer();
  // Wait 5 seconds for a timeout.
  QTimer timer;
  timer.setSingleShot(true);
  timer.start(5000);
  while (timer.isActive() && m_server->m_connections.size() == origConns)
    qApp->processEvents(QEventLoop::AllEvents, 1000);

#ifdef USE_ZERO_MQ
  // The zero mq socket will be left ...
  QCOMPARE(m_server->m_connections.size(), 1);
#else
  QCOMPARE(m_server->m_connections.size(), 0);
#endif
}

void ServerTest::handleMessage_data()
{
  // Load testing jobs:
  m_server->jobManager()->loadJobState(MoleQueue_TESTDATA_DIR "server-ref");


  QTest::addColumn<QString>("requestFile");
  QTest::addColumn<QString>("responseFile");

  // Invalid method
  QTest::newRow("invalidMethod")
      << "server-ref/invalidMethod-request.json"
      << "server-ref/invalidMethod-response.json";

  // listQueues
  QTest::newRow("listQueues")
      << "server-ref/listQueues-request.json"
      << "server-ref/listQueues-response.json";

  // submitJob
  QTest::newRow("submitJob-paramsNotObject")
      << "server-ref/submitJob-paramsNotObject-request.json"
      << "server-ref/submitJob-paramsNotObject-response.json";
  QTest::newRow("submitJob-queueMissing")
      << "server-ref/submitJob-queueMissing-request.json"
      << "server-ref/submitJob-queueMissing-response.json";
  QTest::newRow("submitJob-programMissing")
      << "server-ref/submitJob-programMissing-request.json"
      << "server-ref/submitJob-programMissing-response.json";
  QTest::newRow("submitJob-queueNotString")
      << "server-ref/submitJob-queueNotString-request.json"
      << "server-ref/submitJob-queueNotString-response.json";
  QTest::newRow("submitJob-programNotString")
      << "server-ref/submitJob-programNotString-request.json"
      << "server-ref/submitJob-programNotString-response.json";
  QTest::newRow("submitJob-queueDoesNotExist")
      << "server-ref/submitJob-queueDoesNotExist-request.json"
      << "server-ref/submitJob-queueDoesNotExist-response.json";
  QTest::newRow("submitJob-programDoesNotExist")
      << "server-ref/submitJob-programDoesNotExist-request.json"
      << "server-ref/submitJob-programDoesNotExist-response.json";
  QTest::newRow("submitJob")
      << "server-ref/submitJob-request.json"
      << "server-ref/submitJob-response.json";

  // cancelJob
  QTest::newRow("cancelJob-paramsNotObject")
      << "server-ref/cancelJob-paramsNotObject-request.json"
      << "server-ref/cancelJob-paramsNotObject-response.json";
  QTest::newRow("cancelJob-moleQueueIdMissing")
      << "server-ref/cancelJob-moleQueueIdMissing-request.json"
      << "server-ref/cancelJob-moleQueueIdMissing-response.json";
  QTest::newRow("cancelJob-moleQueueIdInvalid")
      << "server-ref/cancelJob-moleQueueIdInvalid-request.json"
      << "server-ref/cancelJob-moleQueueIdInvalid-response.json";
  QTest::newRow("cancelJob-jobNotRunning")
      << "server-ref/cancelJob-jobNotRunning-request.json"
      << "server-ref/cancelJob-jobNotRunning-response.json";
  QTest::newRow("cancelJob-invalidQueue")
      << "server-ref/cancelJob-invalidQueue-request.json"
      << "server-ref/cancelJob-invalidQueue-response.json";
  QTest::newRow("cancelJob")
      << "server-ref/cancelJob-request.json"
      << "server-ref/cancelJob-response.json";

  // lookupJob
  QTest::newRow("lookupJob-paramsNotObject")
      << "server-ref/lookupJob-paramsNotObject-request.json"
      << "server-ref/lookupJob-paramsNotObject-response.json";
  QTest::newRow("lookupJob-moleQueueIdMissing")
      << "server-ref/lookupJob-moleQueueIdMissing-request.json"
      << "server-ref/lookupJob-moleQueueIdMissing-response.json";
  QTest::newRow("lookupJob-moleQueueIdInvalid")
      << "server-ref/lookupJob-moleQueueIdInvalid-request.json"
      << "server-ref/lookupJob-moleQueueIdInvalid-response.json";
  QTest::newRow("lookupJob")
      << "server-ref/lookupJob-request.json"
      << "server-ref/lookupJob-response.json";
}

void ServerTest::handleMessage()
{
  // Fetch the filenames for this iteration
  QFETCH(QString, requestFile);
  QFETCH(QString, responseFile);

  // Load the json strings
  ReferenceString requestString(requestFile);
  ReferenceString responseString(responseFile);

  // Parse the request into a message
  DummyConnection conn;
  QJsonDocument doc =
      QJsonDocument::fromJson(requestString.toString().toLatin1());
  QVERIFY(doc.isObject());
  Message message(doc.object(), &conn);
  QVERIFY(message.parse());

  // Pass the message to the server for handling:
  m_server->handleMessage(message);

  // Verify that a reply was sent:
  QVERIFY(conn.messageCount() > 0);

  // Compare the reply with the reference reply
  QCOMPARE(QString(conn.popMessage().toJson()), responseString.toString());
}

QTEST_MAIN(ServerTest)

#include "servertest.moc"
