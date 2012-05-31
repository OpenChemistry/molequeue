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

#include "client.h"

#include "jobrequest.h"
#include "molequeueglobal.h"

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

class ClientTest : public QObject
{
  Q_OBJECT

private:
  TestServer *m_server;
  MoleQueue::Client *m_client;
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

  void testJobSubmission();
  void testJobCancellation();
  void testRequestQueueListUpdate();

  void testQueueListReceived();
  void testSuccessfulSubmissionReceived();
  void testFailedSubmissionReceived();
  void testJobCancellationConfirmationReceived();
  void testJobStateChangeReceived();
};


MoleQueue::PacketType ClientTest::readReferenceString(const QString &filename)
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

void ClientTest::initTestCase()
{
  m_server = new TestServer(&m_packet);
  m_client = new MoleQueue::Client();
  m_client->connectToServer();
  // Let the event loop run a bit to handle the connections
  qApp->processEvents(QEventLoop::AllEvents, 1000);
}

void ClientTest::cleanupTestCase()
{
  delete m_server;
  delete m_client;
}

void ClientTest::init()
{
  m_packet.clear();
}

void ClientTest::cleanup()
{
}

void ClientTest::testJobSubmission()
{
  MoleQueue::JobRequest req;

  req.setQueue("Some queue");
  req.setProgram("Some program");
  req.setDescription("Test job");
  req.setInputAsString("I'm a sample input text!");

  m_client->submitJobRequest(req);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  MoleQueue::PacketType refPacket =
      this->readReferenceString("client-ref/job-submission.json");

  // Strip out the random ids in the packets
  QRegExp strip ("\\n\\s+\"id\"\\s+:\\s+\\d+\\s*,\\s*\\n");
  QString strippedPacket = QString(m_packet);
  QString strippedRefPacket = QString(refPacket);

  strippedPacket.replace(strip, "\n");
  strippedRefPacket.replace(strip, "\n");

  QCOMPARE(strippedPacket, strippedRefPacket);
}

void ClientTest::testJobCancellation()
{
  MoleQueue::JobRequest req;
  m_client->cancelJobRequest(req);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  MoleQueue::PacketType refPacket =
      this->readReferenceString("client-ref/job-cancellation.json");

  // Strip out the random ids in the packets
  QRegExp strip ("\\n\\s+\"id\"\\s+:\\s+\\d+\\s*,\\s*\\n");
  QString strippedPacket = QString(m_packet);
  QString strippedRefPacket = QString(refPacket);

  strippedPacket.replace(strip, "\n");
  strippedRefPacket.replace(strip, "\n");

  QCOMPARE(strippedPacket, strippedRefPacket);
}

void ClientTest::testRequestQueueListUpdate()
{
  m_client->requestQueueListUpdate();

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  MoleQueue::PacketType refPacket =
      this->readReferenceString("client-ref/queue-list-request.json");

  // Strip out the random ids in the packets
  QRegExp strip ("\\n\\s+\"id\"\\s+:\\s+\\d+\\s*,\\s*\\n");
  QString strippedPacket = QString(m_packet);
  QString strippedRefPacket = QString(refPacket);

  strippedPacket.replace(strip, "\n");
  strippedRefPacket.replace(strip, "\n");

  QCOMPARE(strippedPacket, strippedRefPacket);
}

void ClientTest::testQueueListReceived()
{
  // First send a listQueues request, then parse out the id for the response.
  m_client->requestQueueListUpdate();

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\\d+)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in queue list request!");
  MoleQueue::IdType id = static_cast<MoleQueue::IdType>(capture.cap(1).toULong());

  QSignalSpy spy (m_client, SIGNAL(queueListUpdated(QueueListType)));

  MoleQueue::PacketType queueList =
      this->readReferenceString("client-ref/queue-list.json");

  queueList.replace("%id%", MoleQueue::PacketType::number(id));

  m_server->sendPacket(queueList);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);

  const MoleQueue::QueueListType signalList =
      spy.first().first().value<MoleQueue::QueueListType>();
  const MoleQueue::QueueListType clientList = m_client->queueList();
  QCOMPARE(signalList, clientList);
  QCOMPARE(signalList.size(), 2);
  QCOMPARE(signalList[0].second.size(), 5);
  QCOMPARE(signalList[1].second.size(), 5);
}

void ClientTest::testSuccessfulSubmissionReceived()
{
  // First send a submitJob request, then parse out the id for the response.
  MoleQueue::JobRequest req;
  m_client->submitJobRequest(req);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\\d+)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in job submission request!");
  MoleQueue::IdType id = static_cast<MoleQueue::IdType>(capture.cap(1).toULong());

  QSignalSpy spy (m_client, SIGNAL(jobSubmitted(JobRequest,bool,QString)));

  MoleQueue::PacketType response =
      this->readReferenceString("client-ref/successful-submission.json");

  response.replace("%id%", MoleQueue::PacketType::number(id));

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 3);

  const bool success = spy.first()[1].toBool();
  const QString err = spy.first()[2].toString();
  QVERIFY(success);
  QVERIFY(err.isEmpty());
}

void ClientTest::testFailedSubmissionReceived()
{
  // First send a submitJob request, then parse out the id for the response.
  MoleQueue::JobRequest req;
  m_client->submitJobRequest(req);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\\d+)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in job submission request!");
  MoleQueue::IdType id = static_cast<MoleQueue::IdType>(capture.cap(1).toULong());

  QSignalSpy spy (m_client, SIGNAL(jobSubmitted(JobRequest,bool,QString)));

  MoleQueue::PacketType response =
      this->readReferenceString("client-ref/failed-submission.json");

  response.replace("%id%", MoleQueue::PacketType::number(id));

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 3);

  const bool success = spy.first()[1].toBool();
  const QString err = spy.first()[2].toString();
  QVERIFY(!success);
  QVERIFY(!err.isEmpty());
}

void ClientTest::testJobCancellationConfirmationReceived()
{
  // First send a cancelJob request, then parse out the id for the response.
  MoleQueue::JobRequest req;
  m_client->submitJobRequest(req);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }
  m_packet.clear();

  m_client->cancelJobRequest(req);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\\d+)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in job cancellation request!");
  MoleQueue::IdType id = static_cast<MoleQueue::IdType>(capture.cap(1).toULong());

  QSignalSpy spy (m_client, SIGNAL(jobCanceled(JobRequest,bool,QString)));

  MoleQueue::PacketType response =
      this->readReferenceString("client-ref/job-canceled.json");

  response.replace("%id%", MoleQueue::PacketType::number(id));

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 3);

  const bool success = spy.first()[1].toBool();
  const QString err = spy.first()[2].toString();
  QVERIFY(success);
  QVERIFY(err.isEmpty());
}

void ClientTest::testJobStateChangeReceived()
{
  QSignalSpy spy (m_client,
                  SIGNAL(jobStateChanged(JobRequest,JobState,JobState)));

  MoleQueue::PacketType response =
      this->readReferenceString("client-ref/jobstate-change.json");

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 3);

  const MoleQueue::JobState before = spy.first()[1].value<MoleQueue::JobState>();
  const MoleQueue::JobState after  = spy.first()[2].value<MoleQueue::JobState>();
  QCOMPARE(before, MoleQueue::RunningRemote);
  QCOMPARE(after,  MoleQueue::Finished);
}

QTEST_MAIN(ClientTest)

#include "moc_clienttest.cxx"
