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

#include "localsocketclient.h"

#include "job.h"
#include "jobmanager.h"
#include "molequeueglobal.h"

#include "testserver.h"

#include <QtGui/QApplication>

#include <QtNetwork/QLocalSocket>

class ZeroMqClientTest : public QObject
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


MoleQueue::PacketType ZeroMqClientTest::readReferenceString(const QString &filename)
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

void ZeroMqClientTest::initTestCase()
{
  m_server = new TestServer(&m_packet);
  m_client = new MoleQueue::LocalSocketClient();
  m_client->connectToServer(m_server->socketName());
  // Let the event loop run a bit to handle the connections
  qApp->processEvents(QEventLoop::AllEvents, 1000);
}

void ZeroMqClientTest::cleanupTestCase()
{
  delete m_server;
  delete m_client;
}

void ZeroMqClientTest::init()
{
  m_packet.clear();
}

void ZeroMqClientTest::cleanup()
{
}

void ZeroMqClientTest::testJobSubmission()
{
  MoleQueue::Job *req = m_client->newJobRequest();

  req->setQueue("Some queue");
  req->setProgram("Some program");
  req->setDescription("Test job");
  req->setInputAsString("I'm a sample input text!");

  m_client->submitJobRequest(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      readReferenceString("client-ref/job-submission.json");

  // Strip out the random ids in the packets
  QRegExp strip ("\\n\\s+\"id\"\\s+:\\s+\\d+\\s*,\\s*\\n");
  QString strippedPacket = QString(m_packet);
  QString strippedRefPacket = QString(refPacket);

  strippedPacket.replace(strip, "\n");
  strippedRefPacket.replace(strip, "\n");

  QCOMPARE(strippedPacket, strippedRefPacket);
}

void ZeroMqClientTest::testJobCancellation()
{
  MoleQueue::Job *req = m_client->newJobRequest();
  m_client->cancelJob(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      readReferenceString("client-ref/job-cancellation.json");

  // Strip out the random ids in the packets
  QRegExp strip ("\\n\\s+\"id\"\\s+:\\s+\\d+\\s*,\\s*\\n");
  QString strippedPacket = QString(m_packet);
  QString strippedRefPacket = QString(refPacket);

  strippedPacket.replace(strip, "\n");
  strippedRefPacket.replace(strip, "\n");

  QCOMPARE(strippedPacket, strippedRefPacket);
}

void ZeroMqClientTest::testRequestQueueListUpdate()
{
  m_client->requestQueueListUpdate();

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  MoleQueue::PacketType refPacket =
      readReferenceString("client-ref/queue-list-request.json");

  // Strip out the random ids in the packets
  QRegExp strip ("\\n\\s+\"id\"\\s+:\\s+\\d+\\s*,\\s*\\n");
  QString strippedPacket = QString(m_packet);
  QString strippedRefPacket = QString(refPacket);

  strippedPacket.replace(strip, "\n");
  strippedRefPacket.replace(strip, "\n");

  QCOMPARE(strippedPacket, strippedRefPacket);
}

void ZeroMqClientTest::testQueueListReceived()
{
  // First send a listQueues request, then parse out the id for the response.
  m_client->requestQueueListUpdate();

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\\d+)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in queue list request!");
  MoleQueue::IdType id = static_cast<MoleQueue::IdType>(capture.cap(1).toULong());

  QSignalSpy spy (m_client, SIGNAL(queueListUpdated(MoleQueue::QueueListType)));

  MoleQueue::PacketType queueList =
      readReferenceString("client-ref/queue-list.json");

  queueList.replace("%id%", MoleQueue::PacketType::number(id));

  m_server->sendPacket(queueList);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);

  const MoleQueue::QueueListType signalList =
      spy.first().first().value<MoleQueue::QueueListType>();
  const MoleQueue::QueueListType clientList = m_client->queueList();
  QCOMPARE(signalList, clientList);
  QCOMPARE(signalList.size(), 2);
}

void ZeroMqClientTest::testSuccessfulSubmissionReceived()
{
  // First send a submitJob request, then parse out the id for the response.
  MoleQueue::Job *req = m_client->newJobRequest();
  m_client->submitJobRequest(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\\d+)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in job submission request!");
  MoleQueue::IdType id = static_cast<MoleQueue::IdType>(capture.cap(1).toULong());

  QSignalSpy spy (m_client, SIGNAL(jobSubmitted(const MoleQueue::Job*,bool,QString)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/successful-submission.json");

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

void ZeroMqClientTest::testFailedSubmissionReceived()
{
  // First send a submitJob request, then parse out the id for the response.
  MoleQueue::Job *req = m_client->newJobRequest();
  m_client->submitJobRequest(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\\d+)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in job submission request!");
  MoleQueue::IdType id = static_cast<MoleQueue::IdType>(capture.cap(1).toULong());

  QSignalSpy spy (m_client, SIGNAL(jobSubmitted(const MoleQueue::Job*,bool,QString)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/failed-submission.json");

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

void ZeroMqClientTest::testJobCancellationConfirmationReceived()
{
  // First send a cancelJob request, then parse out the id for the response.
  MoleQueue::Job *req = m_client->newJobRequest();
  m_client->submitJobRequest(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");
  m_packet.clear();

  m_client->cancelJob(req);

  QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  QRegExp capture ("\\n\\s+\"id\"\\s+:\\s+(\\d+)\\s*,\\s*\\n");
  int pos = capture.indexIn(m_packet);
  QVERIFY2(pos >= 0, "id not found in job cancellation request!");
  MoleQueue::IdType id = static_cast<MoleQueue::IdType>(capture.cap(1).toULong());

  QSignalSpy spy (m_client, SIGNAL(jobCanceled(const MoleQueue::Job*,bool,QString)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/job-canceled.json");

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

void ZeroMqClientTest::testJobStateChangeReceived()
{
  QSignalSpy spy (m_client,
                  SIGNAL(jobStateChanged(const MoleQueue::Job*,MoleQueue::JobState,MoleQueue::JobState)));

  MoleQueue::PacketType response =
      readReferenceString("client-ref/jobstate-change.json");

  // Fake the molequeue id
  MoleQueue::Job *job = m_client->newJobRequest();
  job->setMolequeueId(1);
  m_client->m_jobManager->jobIdsChanged(job);

  m_server->sendPacket(response);

  qApp->processEvents(QEventLoop::AllEvents, 1000);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().size(), 3);

  const MoleQueue::JobState before = spy.first()[1].value<MoleQueue::JobState>();
  const MoleQueue::JobState after  = spy.first()[2].value<MoleQueue::JobState>();
  QCOMPARE(before, MoleQueue::RunningRemote);
  QCOMPARE(after,  MoleQueue::Finished);
}

QTEST_MAIN(ZeroMqClientTest)

#include "moc_clienttest.cxx"
