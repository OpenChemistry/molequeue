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

#include "actionfactorymanager.h"
#include "jobactionfactories/openwithactionfactory.h"
#include "jobmanager.h"
#include "program.h"
#include <molequeue/servercore/localsocketconnectionlistener.h>
#include "testing/dummyconnection.h"
#include "testing/referencestring.h"
#include "testing/testserver.h"
#include "queue.h"
#include "queuemanager.h"

#include <QtWidgets/QApplication>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include <QtCore/QJsonDocument>
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

  void verifyOpenWithHandler();
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

// Simplify the addition of new validation tests. Expects files to exist in
// molequeue/molequeue/testing/data/server-ref/ named:
// - <name>-request.json: a client request.
// - <name>-response.json: a reference server reply.
void addValidation(const QString &name)
{
  QTest::newRow(qPrintable(name))
      << QString("server-ref/%1-request.json").arg(name)
      << QString("server-ref/%1-response.json").arg(name);
}

void ServerTest::handleMessage_data()
{
  // Load testing jobs:
  m_server->jobManager()->loadJobState(MoleQueue_TESTDATA_DIR "server-ref");


  QTest::addColumn<QString>("requestFile");
  QTest::addColumn<QString>("responseFile");

  // Invalid method
  addValidation("invalidMethod");

  // listQueues
  addValidation("listQueues");

  // submitJob
  addValidation("submitJob-paramsNotObject");
  addValidation("submitJob-queueMissing");
  addValidation("submitJob-programMissing");
  addValidation("submitJob-queueNotString");
  addValidation("submitJob-programNotString");
  addValidation("submitJob-queueDoesNotExist");
  addValidation("submitJob-programDoesNotExist");
  addValidation("submitJob");

  // cancelJob
  addValidation("cancelJob-paramsNotObject");
  addValidation("cancelJob-moleQueueIdMissing");
  addValidation("cancelJob-moleQueueIdInvalid");
  addValidation("cancelJob-jobNotRunning");
  addValidation("cancelJob-invalidQueue");
  addValidation("cancelJob");

  // lookupJob
  addValidation("lookupJob-paramsNotObject");
  addValidation("lookupJob-moleQueueIdMissing");
  addValidation("lookupJob-moleQueueIdInvalid");
  addValidation("lookupJob");

  // registerOpenWith
  addValidation("registerOpenWith");
  addValidation("registerOpenWith-rpc");
  addValidation("registerOpenWith-duplicateName"); // Must follow registerOpenWith
  addValidation("registerOpenWith-paramsNotObject");
  addValidation("registerOpenWith-badNameExec");
  addValidation("registerOpenWith-emptyName");
  addValidation("registerOpenWith-patternsNotArray");
  addValidation("registerOpenWith-patternNotObject");
  addValidation("registerOpenWith-invalidPatternType");

  // listOpenWithName
  addValidation("listOpenWithNames");

  // unregisterOpenWith
  addValidation("unregisterOpenWith-prepare"); // add a dummy handler, and
  addValidation("unregisterOpenWith"); // remove it.
  addValidation("unregisterOpenWith-paramsNotObject");
  addValidation("unregisterOpenWith-nameNotString");
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

// Verify that the action factories added by the registerOpenWith test are valid
void ServerTest::verifyOpenWithHandler()
{
  // Get handlers:
  ActionFactoryManager *afm = ActionFactoryManager::instance();
  QList<OpenWithActionFactory*> factories =
      afm->factoriesOfType<OpenWithActionFactory>();
  QCOMPARE(factories.size(), 2);

  // test the executable handler's configuration
  OpenWithActionFactory *spiffyClient = factories.first();
  QVERIFY(spiffyClient != NULL);
  QCOMPARE(spiffyClient->name(), QString("My Spiffy Client"));
  QCOMPARE(spiffyClient->executable(), QString("client"));
  QList<QRegExp> patterns = spiffyClient->filePatterns();
  QCOMPARE(patterns.size(), 2);
  QCOMPARE(patterns[0].pattern(), QString("spiff[\\d]*\\.(?:dat|out)"));
  QCOMPARE(patterns[0].patternSyntax(), QRegExp::RegExp2);
  QCOMPARE(patterns[0].caseSensitivity(), Qt::CaseSensitive);
  QCOMPARE(patterns[1].pattern(), QString("*.spiffyout"));
  QCOMPARE(patterns[1].patternSyntax(), QRegExp::WildcardUnix);
  QCOMPARE(patterns[1].caseSensitivity(), Qt::CaseInsensitive);

  // test the rpc handler's configuration
  spiffyClient = factories.at(1);
  QVERIFY(spiffyClient != NULL);
  QCOMPARE(spiffyClient->name(), QString("My Spiffy Client (RPC)"));
  QCOMPARE(spiffyClient->rpcServer(), QString("rpc-client"));
  patterns = spiffyClient->filePatterns();
  QCOMPARE(patterns.size(), 2);
  QCOMPARE(patterns[0].pattern(), QString("rpcspiff[\\d]*\\.(?:dat|out)"));
  QCOMPARE(patterns[0].patternSyntax(), QRegExp::RegExp2);
  QCOMPARE(patterns[0].caseSensitivity(), Qt::CaseSensitive);
  QCOMPARE(patterns[1].pattern(), QString("rpc*.spiffyout"));
  QCOMPARE(patterns[1].patternSyntax(), QRegExp::WildcardUnix);
  QCOMPARE(patterns[1].caseSensitivity(), Qt::CaseInsensitive);
}

QTEST_MAIN(ServerTest)

#include "servertest.moc"
