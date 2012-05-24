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

#include "serverconnection.h"

#include "jobrequest.h"
#include "molequeueglobal.h"
#include "program.h"
#include "queue.h"
#include "queuemanager.h"

#include <QtGui/QApplication>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

class TestServer : public QObject
{
  Q_OBJECT
  MoleQueue::PacketType *m_target;
  QLocalServer *m_server;
  QLocalSocket *m_socket;
public:
  TestServer(MoleQueue::PacketType *target)
    : QObject(NULL), m_target(target), m_server(new QLocalServer),
      m_socket(NULL)
  {
    if (!m_server->listen("MoleQueue")) {
      qWarning() << "Cannot start test server:" << m_server->errorString();
      return;
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
  }

  ~TestServer()
  {
    if (m_socket != NULL)
      m_socket->disconnect();

    delete m_server;
  }

  void sendPacket(const MoleQueue::PacketType &packet)
  {
    QDataStream out (m_socket);
    out.setVersion(QDataStream::Qt_4_7);
//    qDebug() << "Test server writing" << packet.size() << "bytes.";

    // Create header
    out << static_cast<quint32>(1);
    out << static_cast<quint32>(packet.size());
    // write data
    out << packet;
    m_socket->flush();
  }

private slots:
  void newConnection()
  {
    m_socket = m_server->nextPendingConnection();
    connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
//    qDebug() << "New connection!";
  }

  void readyRead()
  {
//  qDebug() << "Test server received" << m_socket->bytesAvailable() << "bytes.";
    QDataStream in (m_socket);
    in.setVersion(QDataStream::Qt_4_7);

    quint32 version;
    quint32 size;
    MoleQueue::PacketType packet;

    in >> version;
    in >> size;
    in >> packet;

//    qDebug() << "Received packet. Version" << version << "Size" << size;

    m_target->append(packet);
  }
};

class ServerConnectionTest : public QObject
{
  Q_OBJECT

private:
  TestServer *m_server;
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
  m_server = new TestServer(&m_packet);
  QLocalSocket *serverSocket = new QLocalSocket ();
  serverSocket->connectToServer("MoleQueue");
  m_serverConnection = new MoleQueue::ServerConnection(NULL, serverSocket);
  m_serverConnection->startProcessing();
  // Let the event loop run a bit to handle the connections
  qApp->processEvents(QEventLoop::AllEvents, 1000);
}

void ServerConnectionTest::cleanupTestCase()
{
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
  MoleQueue::Queue *queueTmp = qmanager.createQueue("Remote - SGE");
  qmanager.addQueue(queueTmp);
  queueTmp->setName("Some big ol' cluster");
  MoleQueue::Program *progTmp = new MoleQueue::Program (NULL);
  progTmp->setName("Quantum Tater");
  queueTmp->addProgram(progTmp);
  progTmp = new MoleQueue::Program (*progTmp);
  progTmp->setName("Crystal Math");
  queueTmp->addProgram(progTmp);
  progTmp = new MoleQueue::Program (*progTmp);
  progTmp->setName("Nebulous Nucleus");
  queueTmp->addProgram(progTmp);
  queueTmp = qmanager.createQueue("Local");
  qmanager.addQueue(queueTmp);
  queueTmp->setName("Puny local queue");
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

  m_serverConnection->sendQueueList(&qmanager);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  // Cleanup
  const QList<MoleQueue::Queue*> queues = qmanager.queues();
  for (QList<MoleQueue::Queue*>::const_iterator it = queues.constBegin(),
       it_end = queues.constEnd(); it != it_end; ++it) {
    foreach (const QString &prog, (*it)->programs()) {
      delete (*it)->program(prog);
    }
  }
  qDeleteAll(qmanager.queues());

  MoleQueue::PacketType refPacket =
      this->readReferenceString("serverconnection-ref/queue-list.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void ServerConnectionTest::testSendSuccessfulSubmissionResponse()
{
  MoleQueue::JobRequest req;

  req.setLocalWorkingDirectory("/tmp/some/path");
  req.setMolequeueId(23);
  req.setClientId(2);
  req.setQueueJobId(1439932);

  // Fake the request
  m_serverConnection->jobSubmissionRequestReceived(92, req.hash());

  // Send the reply
  m_serverConnection->sendSuccessfulSubmissionResponse(req);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  MoleQueue::PacketType refPacket =
      this->readReferenceString("serverconnection-ref/submit-success.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void ServerConnectionTest::testSendFailedSubmissionResponse()
{
  MoleQueue::JobRequest req;

  // Fake the request
  m_serverConnection->jobSubmissionRequestReceived(92, req.hash());

  // Send the reply
  m_serverConnection->sendFailedSubmissionResponse(req, MoleQueue::Success,
                                                   "Not a real error!");

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  MoleQueue::PacketType refPacket =
      this->readReferenceString("serverconnection-ref/submit-failure.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void ServerConnectionTest::testSendSuccessfulCancellationResponse()
{
  MoleQueue::JobRequest req;
  req.setMolequeueId(21);

  // Fake the request
  m_serverConnection->jobCancellationRequestReceived(93, req.moleQueueId());

  // Send the reply
  m_serverConnection->sendSuccessfulCancellationResponse(req);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  MoleQueue::PacketType refPacket =
      this->readReferenceString("serverconnection-ref/cancel-success.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void ServerConnectionTest::testJobStateChangeNotification()
{
  MoleQueue::JobRequest req;
  req.setMolequeueId(15);

  m_serverConnection->sendJobStateChangeNotification(
        req, MoleQueue::RunningLocal, MoleQueue::Finished);


  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  MoleQueue::PacketType refPacket =
      this->readReferenceString("serverconnection-ref/state-change.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void ServerConnectionTest::testQueueListRequested()
{
  QSignalSpy spy (m_serverConnection, SIGNAL(queueListRequested()));

  MoleQueue::PacketType response =
      this->readReferenceString("serverconnection-ref/queue-list-request.json");

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
}

void ServerConnectionTest::testJobSubmissionRequested()
{
  QSignalSpy spy (m_serverConnection,
                  SIGNAL(jobSubmissionRequested(JobRequest)));

  MoleQueue::PacketType response =
      this->readReferenceString("serverconnection-ref/job-request.json");

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 1);

  MoleQueue::JobRequest req = spy.first().first().value<MoleQueue::JobRequest>();
  QCOMPARE(req.description(), QString("spud slicer 28"));
}

void ServerConnectionTest::testJobCancellationRequested()
{
  QSignalSpy spy (m_serverConnection,
                  SIGNAL(jobCancellationRequested(IdType)));

  MoleQueue::PacketType response =
      this->readReferenceString("serverconnection-ref/job-cancellation.json");

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 1);

  MoleQueue::IdType moleQueueId = spy.first().first().value<MoleQueue::IdType>();
  QCOMPARE(moleQueueId, static_cast<MoleQueue::IdType>(0));
}

QTEST_MAIN(ServerConnectionTest)

#include "moc_serverconnectiontest.cxx"
