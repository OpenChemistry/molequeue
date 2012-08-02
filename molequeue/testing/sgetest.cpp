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

#include "queues/sge.h"

class QueueSgeTest : public QObject
{
  Q_OBJECT

private:
  MoleQueue::QueueSge m_queue;

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void sanityCheck();
  void testParseJobId();
  void testParseQueueLine();
};

void QueueSgeTest::initTestCase()
{
}

void QueueSgeTest::cleanupTestCase()
{
}

void QueueSgeTest::init()
{
}

void QueueSgeTest::cleanup()
{
}

void QueueSgeTest::sanityCheck()
{
  QCOMPARE(m_queue.typeName(), QString("Sun Grid Engine"));
  QString testString = "some.host.somewhere";
  m_queue.setHostName(testString);
  QCOMPARE(m_queue.hostName(), testString);
  testString = "aUser";
  m_queue.setUserName(testString);
  QCOMPARE(m_queue.userName(), testString);
  m_queue.setSshPort(6887);
  QCOMPARE(m_queue.sshPort(), 6887);
  testString = "/some/path";
  m_queue.setWorkingDirectoryBase(testString);
  QCOMPARE(m_queue.workingDirectoryBase(), testString);
  testString = "subComm";
  m_queue.setSubmissionCommand(testString);
  QCOMPARE(m_queue.submissionCommand(), testString);
  testString = "reqComm";
  m_queue.setRequestQueueCommand(testString);
  QCOMPARE(m_queue.requestQueueCommand(), testString);
}

void QueueSgeTest::testParseJobId()
{
  QString submissionOutput = "your job 1235 ('someFile') has been submitted";
  MoleQueue::IdType jobId;
  QVERIFY(m_queue.parseQueueId(submissionOutput, &jobId));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(1235));
}

void QueueSgeTest::testParseQueueLine()
{
  QString line;
  MoleQueue::IdType jobId;
  MoleQueue::JobState state;

  // First some invalid lines
  line = "job-ID   prior   name         user      state   submit/start at     queue      function";
  QVERIFY(!m_queue.parseQueueLine(line, &jobId, &state));
  line = "                                                20:27:15";
  QVERIFY(!m_queue.parseQueueLine(line, &jobId, &state));
  line = "230      0       hydra        craig     inv     07/13/96            durin.q    MASTER";
  QVERIFY(!m_queue.parseQueueLine(line, &jobId, &state));

  // Check various states:
  line = "231      0       hydra        craig     r       07/13/96            durin.q    MASTER";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(231));
  QCOMPARE(state, MoleQueue::RunningRemote);

  line = "232      0       hydra        craig     d       07/13/96            durin.q    MASTER";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(232));
  QCOMPARE(state, MoleQueue::RunningRemote);

  line = "233      0       hydra        craig     e       07/13/96            durin.q    MASTER";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(233));
  QCOMPARE(state, MoleQueue::RunningRemote);

  line = "234      0       hydra        craig     qw       07/13/96            durin.q    MASTER";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(234));
  QCOMPARE(state, MoleQueue::RemoteQueued);

  line = "235      0       hydra        craig     q       07/13/96            durin.q    MASTER";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(235));
  QCOMPARE(state, MoleQueue::RemoteQueued);

  line = "236      0       hydra        craig     w       07/13/96            durin.q    MASTER";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(236));
  QCOMPARE(state, MoleQueue::RemoteQueued);

  line = "237      0       hydra        craig     s       07/13/96            durin.q    MASTER";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(237));
  QCOMPARE(state, MoleQueue::RemoteQueued);

  line = "238      0       hydra        craig     h       07/13/96            durin.q    MASTER";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(238));
  QCOMPARE(state, MoleQueue::RemoteQueued);

  line = "239      0       hydra        craig     t       07/13/96            durin.q    MASTER";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(239));
  QCOMPARE(state, MoleQueue::RemoteQueued);

}

QTEST_MAIN(QueueSgeTest)

#include "sgetest.moc"
