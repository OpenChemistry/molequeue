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
#include "testserver.h"

#include "serverconnection.h"

#include "job.h"
#include "jobmanager.h"
#include "molequeueglobal.h"
#include "program.h"
#include "queue.h"
#include "queuemanager.h"
#include "server.h"

#include <QtNetwork/QLocalSocket>

class QueueDummy : public MoleQueue::Queue
{
  Q_OBJECT
public:
  QueueDummy(MoleQueue::QueueManager *parentManager)
    : MoleQueue::Queue ("Dummy", parentManager)
  {
  }
public slots:
  bool submitJob(const MoleQueue::Job *) {return false;}
};

class ServerConnectionTest : public QObject
{
  Q_OBJECT

private:
  TestServer *m_testServer;
  MoleQueue::Server *m_server;
  MoleQueue::ServerConnection *m_serverConnection;
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

  // ServerConnection slots
  void testSendQueueList();
  void testSendSuccessfulSubmissionResponse();
  void testSendFailedSubmissionResponse();
  void testSendSuccessfulCancellationResponse();
  void testJobStateChangeNotification();

  // ServerConnection signals
  void testQueueListRequested();
  void testJobSubmissionRequested();
  void testJobCancellationRequested();
};


MoleQueue::PacketType ServerConnectionTest::readReferenceString(const QString &filename)
{
  QString realFilename = TESTDATADIR + filename;
  QFile refFile (realFilename);
  if (!refFile.open(QFile::ReadOnly)) {
    qDebug() << "Cannot access reference file" << realFilename;
    return "";
  }
  MoleQueue::PacketType contents = refFile.readAll();
  refFile.close();
  return contents;
}

void ServerConnectionTest::initTestCase()
{
  m_testServer = new TestServer(&m_packet);

  m_server = new MoleQueue::Server ();
  QLocalSocket *serverSocket = new QLocalSocket ();
  serverSocket->connectToServer(m_testServer->socketName());
  m_serverConnection = new MoleQueue::ServerConnection(m_server, serverSocket);
  m_serverConnection->startProcessing();
  // Let the event loop run a bit to handle the connections
  qApp->processEvents(QEventLoop::AllEvents, 1000);
}

void ServerConnectionTest::cleanupTestCase()
{
  delete m_testServer;
  delete m_server;
  delete m_serverConnection;
}

void ServerConnectionTest::init()
{
  m_packet.clear();
}

void ServerConnectionTest::cleanup()
{
}

void ServerConnectionTest::testSendQueueList()
{
  // Create phony queue
  MoleQueue::QueueManager qmanager;
  MoleQueue::Queue *queueTmp = qmanager.addQueue("Some big ol' cluster",
                                                 "Sun Grid Engine");
  MoleQueue::Program *progTmp = new MoleQueue::Program (NULL);
  progTmp->setName("Quantum Tater");
  queueTmp->addProgram(progTmp);
  progTmp = new MoleQueue::Program (*progTmp);
  progTmp->setName("Crystal Math");
  queueTmp->addProgram(progTmp);
  progTmp = new MoleQueue::Program (*progTmp);
  progTmp->setName("Nebulous Nucleus");
  queueTmp->addProgram(progTmp);

  queueTmp = qmanager.addQueue("Puny local queue", "Local");
  progTmp = new MoleQueue::Program (NULL);
  progTmp->setName("SpectroCrunch");
  queueTmp->addProgram(progTmp);
  progTmp = new MoleQueue::Program (*progTmp);
  progTmp->setName("FastFocker");
  queueTmp->addProgram(progTmp);
  progTmp = new MoleQueue::Program (*progTmp);
  progTmp->setName("SpeedSlater");
  queueTmp->addProgram(progTmp);

  // Send dummy request to give the ServerConnection a packetId
  m_serverConnection->queueListRequestReceived(23);

  m_serverConnection->sendQueueList(qmanager.toQueueList());

  QVERIFY2(m_testServer->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      this->readReferenceString("serverconnection-ref/queue-list.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void ServerConnectionTest::testSendSuccessfulSubmissionResponse()
{
  MoleQueue::Job req;

  req.setLocalWorkingDirectory("/tmp/some/path");
  req.setMolequeueId(1);
  req.setClientId(2);
  req.setQueueJobId(1439932);

  // Fake the request
  m_serverConnection->jobSubmissionRequestReceived(92, req.hash());

  // Send the reply
  m_serverConnection->sendSuccessfulSubmissionResponse(&req);

  QVERIFY2(m_testServer->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      this->readReferenceString("serverconnection-ref/submit-success.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void ServerConnectionTest::testSendFailedSubmissionResponse()
{
  MoleQueue::Job req;

  // Fake the request
  m_serverConnection->jobSubmissionRequestReceived(92, req.hash());

  // Get the molequeue id of the submitted job
  MoleQueue::IdType mqId =
      m_serverConnection->m_server->jobManager()->jobs().last()->moleQueueId();
  req.setMolequeueId(mqId);

  // Send the reply
  m_serverConnection->sendFailedSubmissionResponse(&req, MoleQueue::Success,
                                                   "Not a real error!");

  QVERIFY2(m_testServer->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      this->readReferenceString("serverconnection-ref/submit-failure.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void ServerConnectionTest::testSendSuccessfulCancellationResponse()
{
  MoleQueue::Job req;
  req.setMolequeueId(21);

  // Fake the request
  m_serverConnection->jobCancellationRequestReceived(93, req.moleQueueId());

  // Send the reply
  m_serverConnection->sendSuccessfulCancellationResponse(&req);

  QVERIFY2(m_testServer->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      this->readReferenceString("serverconnection-ref/cancel-success.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void ServerConnectionTest::testJobStateChangeNotification()
{
  MoleQueue::Job req;
  req.setMolequeueId(15);

  m_serverConnection->sendJobStateChangeNotification(
        &req, MoleQueue::RunningLocal, MoleQueue::Finished);


  QVERIFY2(m_testServer->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      this->readReferenceString("serverconnection-ref/state-change.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void ServerConnectionTest::testQueueListRequested()
{
  QSignalSpy spy (m_serverConnection, SIGNAL(queueListRequested()));

  MoleQueue::PacketType response =
      this->readReferenceString("serverconnection-ref/queue-list-request.json");

  m_testServer->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
}

void ServerConnectionTest::testJobSubmissionRequested()
{
  QSignalSpy spy (m_serverConnection,
                  SIGNAL(jobSubmissionRequested(const MoleQueue::Job*)));

  MoleQueue::PacketType response =
      this->readReferenceString("serverconnection-ref/job-request.json");

  m_testServer->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 1);

  const MoleQueue::Job *req = spy.first().first().value<const MoleQueue::Job*>();
  QCOMPARE(req->description(), QString("spud slicer 28"));
}

void ServerConnectionTest::testJobCancellationRequested()
{
  QSignalSpy spy (m_serverConnection,
                  SIGNAL(jobCancellationRequested(MoleQueue::IdType)));

  MoleQueue::PacketType response =
      this->readReferenceString("serverconnection-ref/job-cancellation.json");

  m_testServer->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 1);

  MoleQueue::IdType moleQueueId = spy.first().first().value<MoleQueue::IdType>();
  QCOMPARE(moleQueueId, static_cast<MoleQueue::IdType>(0));
}

QTEST_MAIN(ServerConnectionTest)

#include "moc_serverconnectiontest.cxx"
