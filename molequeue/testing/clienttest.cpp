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

#include "filespecification.h"
#include "job.h"
#include "jobrequest.h"
#include "jobmanager.h"
#include "molequeueglobal.h"
#include "molequeuetestconfig.h"
#include "transport/localsocket/localsocketclient.h"

#include "testserver.h"

#include <QtGui/QApplication>

#include <QtNetwork/QLocalSocket>

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
  void testLookupJob();
  void testRequestQueueListUpdate();

  void testQueueListReceived();
  void testSuccessfulSubmissionReceived();
  void testFailedSubmissionReceived();
  void testJobCancellationConfirmationReceived();
  void testJobCancellationErrorReceived();
  void testLookupJobResponseReceived();
  void testLookupJobErrorReceived();
  void testJobStateChangeReceived();
};


MoleQueue::PacketType ClientTest::readReferenceString(const QString &filename)
{
  QString realFilename = MoleQueue_TESTDATA_DIR + filename;
  QFile refFile (realFilename);
  if (!refFile.open(QFile::ReadOnly | QFile::Text)) {
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
  m_client = new MoleQueue::LocalSocketClient(this);
  qDebug() << m_server->socketName();
  m_client->connectToServer(m_server->socketName());
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
  MoleQueue::JobRequest req = m_client->newJobRequest();

  req.setQueue("Some queue");
  req.setProgram("Some program");
  req.setDescription("Test job");
  req.setInputFile(MoleQueue::FileSpecification("file.ext",
                                                "I'm a sample input text!"));

  m_client->submitJobRequest(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      readReferenceString("client-ref/job-submission.json");

  // Strip out the random ids in the packets
  QRegExp strip ("\\n\\s+\"id\"\\s+:\\s+\"?\\d+\"?\\s*,\\s*\\n");
  QString strippedPacket = QString(m_packet);
  QString strippedRefPacket = QString(refPacket);

  strippedPacket.replace(strip, "\n");
  strippedRefPacket.replace(strip, "\n");

  QCOMPARE(strippedPacket, strippedRefPacket);
}

void ClientTest::testJobCancellation()
{
  MoleQueue::JobRequest req = m_client->newJobRequest();
  m_client->cancelJob(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      readReferenceString("client-ref/job-cancellation.json");

  // Strip out the random ids in the packets
  QRegExp strip("\\n\\s+\"id\"\\s+:\\s+\"?\\d+\"?\\s*,\\s*\\n");
  QString strippedPacket = QString(m_packet);
  QString strippedRefPacket = QString(refPacket);

  strippedPacket.replace(strip, "\n");
  strippedRefPacket.replace(strip, "\n");

  QCOMPARE(strippedPacket, strippedRefPacket);
}

void ClientTest::testLookupJob()
{
  m_client->lookupJob(12);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      readReferenceString("client-ref/lookupJob-request.json");

  // Strip out the random ids in the packets
  QRegExp strip ("\\n\\s+\"id\"\\s+:\\s+\"?\\d+\"?\\s*,\\s*\\n");
  QString strippedPacket = QString(m_packet);
  QString strippedRefPacket = QString(refPacket);

  strippedPacket.replace(strip, "\n");
  strippedRefPacket.replace(strip, "\n");

  QCOMPARE(strippedPacket, strippedRefPacket);
}

void ClientTest::testRequestQueueListUpdate()
{
  m_client->requestQueueListUpdate();

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      readReferenceString("client-ref/queue-list-request.json");

  // Strip out the random ids in the packets
  QRegExp strip ("\\n\\s+\"id\"\\s+:\\s+\"?\\d+\"?\\s*,\\s*\\n");
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

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\"?\\d+\"?)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in queue list request!");
  QByteArray id = capture.cap(1).toLocal8Bit();

  QSignalSpy spy (m_client, SIGNAL(queueListUpdated(MoleQueue::QueueListType)));

  MoleQueue::PacketType queueList =
      readReferenceString("client-ref/queue-list.json");

  queueList.replace("%id%", id);

  m_server->sendPacket(queueList);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);

  const MoleQueue::QueueListType signalList =
      spy.first().first().value<MoleQueue::QueueListType>();
  const MoleQueue::QueueListType clientList = m_client->queueList();
  QCOMPARE(signalList, clientList);
  QCOMPARE(signalList.size(), 2);
}

void ClientTest::testSuccessfulSubmissionReceived()
{
  // First send a submitJob request, then parse out the id for the response.
  MoleQueue::JobRequest req = m_client->newJobRequest();
  m_client->submitJobRequest(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\"?\\d+\"?)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in job submission request!");
  QByteArray id = capture.cap(1).toLocal8Bit();

  QSignalSpy spy (m_client, SIGNAL(jobSubmitted(MoleQueue::JobRequest,
                                                bool, QString)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/successful-submission.json");

  response.replace("%id%", id);

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
  MoleQueue::JobRequest req = m_client->newJobRequest();
  m_client->submitJobRequest(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\"?\\d+\"?)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in job submission request!");
  QByteArray id = capture.cap(1).toLocal8Bit();

  QSignalSpy spy (m_client, SIGNAL(jobSubmitted(MoleQueue::JobRequest,
                                                bool,QString)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/failed-submission.json");

  response.replace("%id%", id);

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
  MoleQueue::JobRequest req = m_client->newJobRequest();
  m_client->submitJobRequest(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");
  m_packet.clear();

  m_client->cancelJob(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\"?\\d+\"?)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in job cancellation request!");
  QByteArray id = capture.cap(1).toLocal8Bit();

  QSignalSpy spy (m_client, SIGNAL(jobCanceled(MoleQueue::JobRequest,
                                               bool, QString)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/job-canceled.json");

  response.replace("%id%", id);

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 3);

  const bool success = spy.first()[1].toBool();
  const QString err = spy.first()[2].toString();
  QVERIFY(success);
  QVERIFY(err.isEmpty());
}

void ClientTest::testJobCancellationErrorReceived()
{
  // First send a cancelJob request, then parse out the id for the response.
  MoleQueue::JobRequest req = m_client->newJobRequest();
  m_client->submitJobRequest(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");
  m_packet.clear();

  m_client->cancelJob(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\"?\\d+\"?)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in job cancellation request!");
  QByteArray id = capture.cap(1).toLocal8Bit();

  QSignalSpy spy (m_client, SIGNAL(jobCanceled(MoleQueue::JobRequest,
                                               bool, QString)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/job-canceled.json");

  response.replace("%id%", id);

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 3);

  const bool success = spy.first()[1].toBool();
  const QString err = spy.first()[2].toString();
  QVERIFY(success);
  QVERIFY(err.isEmpty());
}

void ClientTest::testLookupJobResponseReceived()
{
  // First send a lookupJob request, then parse out the id for the response.
  const MoleQueue::IdType moleQueueId = 17;
  m_client->lookupJob(moleQueueId);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\"?\\d+\"?)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in lookupJob request!");
  const QByteArray packetId = capture.cap(1).toLocal8Bit();

  QSignalSpy spy(m_client, SIGNAL(lookupJobComplete(MoleQueue::JobRequest,
                                                    MoleQueue::IdType)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/lookupJob-response.json");

  response.replace("%id%", packetId);

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 2);

  MoleQueue::JobRequest req = spy.first()[0].value<MoleQueue::JobRequest>();
  MoleQueue::IdType sigMQId = spy.first()[1].value<MoleQueue::IdType>();
  QVERIFY(req.isValid());
  QCOMPARE(sigMQId, moleQueueId);
}

void ClientTest::testLookupJobErrorReceived()
{
  // First send a lookupJob request, then parse out the id for the response.
  const MoleQueue::IdType moleQueueId = 18;
  m_client->lookupJob(moleQueueId);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture("\\n\\s+\"id\"\\s+:\\s+(\"?\\d+\"?)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in lookupJob request!");
  const QByteArray packetId = capture.cap(1).toLocal8Bit();

  QSignalSpy spy(m_client, SIGNAL(lookupJobComplete(MoleQueue::JobRequest,
                                                    MoleQueue::IdType)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/lookupJob-error.json");

  response.replace("%id%", packetId);

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 2);

  MoleQueue::JobRequest req = spy.first()[0].value<MoleQueue::JobRequest>();
  MoleQueue::IdType sigMQId = spy.first()[1].value<MoleQueue::IdType>();
  QVERIFY(!req.isValid());
  QCOMPARE(sigMQId, moleQueueId);
}

void ClientTest::testJobStateChangeReceived()
{
  QSignalSpy spy (m_client,
                  SIGNAL(jobStateChanged(const MoleQueue::JobRequest &,
                                         MoleQueue::JobState,
                                         MoleQueue::JobState)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/jobstate-change.json");

  // Fake the molequeue id
  MoleQueue::JobRequest job = m_client->newJobRequest();
  MoleQueue::Job(job).setMoleQueueId(1);

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

#include "clienttest.moc"
