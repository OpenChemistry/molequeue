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

#include "queues/pbs.h"

class QueuePbsTest : public QObject
{
  Q_OBJECT

private:
  MoleQueue::QueuePbs m_queue;

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

void QueuePbsTest::initTestCase()
{
}

void QueuePbsTest::cleanupTestCase()
{
}

void QueuePbsTest::init()
{
}

void QueuePbsTest::cleanup()
{
}

void QueuePbsTest::sanityCheck()
{
  QCOMPARE(m_queue.typeName(), QString("PBS/Torque"));
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

void QueuePbsTest::testParseJobId()
{
  QString submissionOutput = "1234.not.a.real.host";
  MoleQueue::IdType jobId;
  QVERIFY(m_queue.parseQueueId(submissionOutput, &jobId));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(1234));
}

void QueuePbsTest::testParseQueueLine()
{
  QString line;
  MoleQueue::IdType jobId;
  MoleQueue::JobState state;

  // First some invalid lines
  line = "Job id           Name             User             Time Use S Queue";
  QVERIFY(!m_queue.parseQueueLine(line, &jobId, &state));
  line = "---------------- ---------------- ---------------- -------- - -----";
  QVERIFY(!m_queue.parseQueueLine(line, &jobId, &state));
  // "I" for "I"nvalid status (doesn't exist in PBS)
  line = "4807.host        scatter          user01           12:56:34 I batch";
  QVERIFY(!m_queue.parseQueueLine(line, &jobId, &state));

  // Check various states:
  line = "231.host         scatter          user01           12:56:34 R batch";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(231));
  QCOMPARE(state, MoleQueue::RunningRemote);

  line = "232.host         scatter          user01           12:56:34 E batch";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(232));
  QCOMPARE(state, MoleQueue::RunningRemote);

  line = "233.host         scatter          user01           12:56:34 C batch";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(233));
  QCOMPARE(state, MoleQueue::RunningRemote);

  line = "234.host         scatter          user01           12:56:34 Q batch";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(234));
  QCOMPARE(state, MoleQueue::RemoteQueued);

  line = "235.host         scatter          user01           12:56:34 H batch";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(235));
  QCOMPARE(state, MoleQueue::RemoteQueued);

  line = "236.host         scatter          user01           12:56:34 T batch";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(236));
  QCOMPARE(state, MoleQueue::RemoteQueued);

  line = "237.host         scatter          user01           12:56:34 W batch";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(237));
  QCOMPARE(state, MoleQueue::RemoteQueued);

  line = "238.host         scatter          user01           12:56:34 S batch";
  QVERIFY(m_queue.parseQueueLine(line, &jobId, &state));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(238));
  QCOMPARE(state, MoleQueue::RemoteQueued);
}

QTEST_MAIN(QueuePbsTest)

#include "moc_pbstest.cxx"
