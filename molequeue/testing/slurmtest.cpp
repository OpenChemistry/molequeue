/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include <QtTest>

#include "queues/slurm.h"

class QueueSlurmTest : public QObject
{
  Q_OBJECT

private:
  MoleQueue::QueueSlurm m_queue;

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
  void testParseQueueLine_data();
  void testParseQueueLine();
};

void QueueSlurmTest::initTestCase()
{
}

void QueueSlurmTest::cleanupTestCase()
{
}

void QueueSlurmTest::init()
{
}

void QueueSlurmTest::cleanup()
{
}

void QueueSlurmTest::sanityCheck()
{
  QCOMPARE(m_queue.typeName(), QString("SLURM"));
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

void QueueSlurmTest::testParseJobId()
{
  QString submissionOutput = "Submitted batch job 1234";
  MoleQueue::IdType jobId;
  QVERIFY(m_queue.parseQueueId(submissionOutput, &jobId));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(1234));
}

void QueueSlurmTest::testParseQueueLine_data()
{
  QTest::addColumn<QString>("data");
  QTest::addColumn<bool>("canParse");
  QTest::addColumn<MoleQueue::IdType>("jobId");
  QTest::addColumn<MoleQueue::JobState>("state");

  QTest::newRow("Header")
      << "JOBID PARTITION     NAME     USER  ST   TIME  NODES NODELIST(REASON)"
      << false
      << MoleQueue::InvalidId
      << MoleQueue::Unknown;

  QTest::newRow("Status: Cancelled, leading whitespace")
      << " 231     debug job2 dave CA   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RunningRemote;

  QTest::newRow("Status: Cancelled, no leading whitespace")
      << "231     debug job2 dave CA   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RunningRemote;

  QTest::newRow("Status: Completed")
      << "231     debug job2 dave CD   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RunningRemote;

  QTest::newRow("Status: Configuring")
      << "231     debug job2 dave CF   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RemoteQueued;

  QTest::newRow("Status: Completing")
      << "231     debug job2 dave CG   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RunningRemote;

  QTest::newRow("Status: Failed")
      << "231     debug job2 dave F   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RunningRemote;

  QTest::newRow("Status: Node fail")
      << "231     debug job2 dave NF   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RunningRemote;

  QTest::newRow("Status: Pending")
      << "231     debug job2 dave PD   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RemoteQueued;

  QTest::newRow("Status: Running")
      << "231     debug job2 dave R   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RunningRemote;

  QTest::newRow("Status: Suspended")
      << "231     debug job2 dave R   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RunningRemote;

  QTest::newRow("Status: Timeout")
      << "231     debug job2 dave TO   0:00     8 (Resources)"
      << true
      << static_cast<MoleQueue::IdType>(231)
      << MoleQueue::RunningRemote;
}

void QueueSlurmTest::testParseQueueLine()
{
  QFETCH(QString, data);
  QFETCH(bool, canParse);
  QFETCH(MoleQueue::IdType, jobId);
  QFETCH(MoleQueue::JobState, state);

  MoleQueue::IdType parsedJobId;
  MoleQueue::JobState parsedState;

  QCOMPARE(m_queue.parseQueueLine(data, &parsedJobId, &parsedState), canParse);
  QCOMPARE(parsedJobId, jobId);
  QCOMPARE(parsedState, state);
}

QTEST_MAIN(QueueSlurmTest)

#include "slurmtest.moc"
