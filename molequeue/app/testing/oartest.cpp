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

#include "queues/oar.h"

class QueueOarTest : public QObject
{
  Q_OBJECT

private:
  MoleQueue::QueueOar m_queue;

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

void QueueOarTest::initTestCase()
{
}

void QueueOarTest::cleanupTestCase()
{
}

void QueueOarTest::init()
{
}

void QueueOarTest::cleanup()
{
}

void QueueOarTest::sanityCheck()
{
  QCOMPARE(m_queue.typeName(), QString("OAR"));
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

void QueueOarTest::testParseJobId()
{
  QString submissionOutput = "SSH finished (94739270797056) Exit code: 0\n[ADMISSION RULE] Modify resource description with type constraints\n[ADMISSION RULE] Automatically add the constraint to go on the 'intuidoc' and 'none' dedicated nodes.\n[ADMISSION_RULE] Resources properties : \\{'property' => '(type = \\'default\\') AND max_walltime >= 5','resources' => [{'value' => '1','resource' => 'core'}]}\n[ADMISSION RULE] Job properties : ((((desktop_computing = 'NO') AND maintenance = 'NO') AND interactive = 'MIXED') AND dedicated IN ('intuidoc','none')) AND gpu = 'NO'\nGenerate a job key...\nOAR_JOB_ID=8160421\n";
  MoleQueue::IdType jobId;
  QVERIFY(m_queue.parseQueueId(submissionOutput, &jobId));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(8160421));

  submissionOutput = "SSH finished (94739270797056) Exit code: 0\n[ADMISSION RULE] Modify resource description with type constraints\n[ADMISSION RULE] Automatically add the constraint to go on the 'intuidoc' and 'none' dedicated nodes.\n[ADMISSION_RULE] Resources properties : \\{'property' => '(type = \\'default\\') AND max_walltime >= 5','resources' => [{'value' => '1','resource' => 'core'}]}\n[ADMISSION RULE] Job properties : ((((desktop_computing = 'NO') AND maintenance = 'NO') AND interactive = 'MIXED') AND dedicated IN ('intuidoc','none')) AND gpu = 'NO'\nGenerate a job key...\nOAR_JOB_ID=816042\n";
  QVERIFY(m_queue.parseQueueId(submissionOutput, &jobId));
  QCOMPARE(jobId, static_cast<MoleQueue::IdType>(816042));
}

void QueueOarTest::testParseQueueLine_data()
{
  QTest::addColumn<QString>("data");
  QTest::addColumn<bool>("canParse");
  QTest::addColumn<MoleQueue::IdType>("jobId");
  QTest::addColumn<MoleQueue::JobState>("state");

  QTest::newRow("Header")
      << "Job id    S User     Duration   System message"
      << false
      << MoleQueue::InvalidId
      << MoleQueue::Unknown;

  QTest::newRow("Status: Accepted, leading whitespace")
      << " 8160394   L kchoi       0:01:18 R=1,W=0:10:0,J=B (Karma=0.000)"
      << true
      << static_cast<MoleQueue::IdType>(8160394)
      << MoleQueue::Accepted;

  QTest::newRow("Status: Accepted, no leading whitespace")
      << "8160394   L kchoi       0:01:18 R=1,W=0:10:0,J=B (Karma=0.000)"
      << true
      << static_cast<MoleQueue::IdType>(8160394)
      << MoleQueue::Accepted;

  QTest::newRow("Status: Error")
      << "8160394   E kchoi       0:01:18 R=1,W=0:10:0,J=B (Karma=0.000)"
      << true
      << static_cast<MoleQueue::IdType>(8160394)
      << MoleQueue::Error;


  QTest::newRow("Status: Submitted")
      << "8160394   W kchoi       0:01:18 R=1,W=0:10:0,J=B (Karma=0.000)"
      << true
      << static_cast<MoleQueue::IdType>(8160394)
      << MoleQueue::Submitted;

  QTest::newRow("Status: RunningRemote")
      << "8160394   R kchoi       0:01:18 R=1,W=0:10:0,J=B (Karma=0.000)"
      << true
      << static_cast<MoleQueue::IdType>(8160394)
      << MoleQueue::RunningRemote;

  QTest::newRow("Status: Finished")
      << "8160394   T kchoi       0:01:18 R=1,W=0:10:0,J=B (Karma=0.000)"
      << true
      << static_cast<MoleQueue::IdType>(8160394)
      << MoleQueue::Finished;

  QTest::newRow("Status: Finished")
      << "8160394   F kchoi       0:01:18 R=1,W=0:10:0,J=B (Karma=0.000)"
      << true
      << static_cast<MoleQueue::IdType>(8160394)
      << MoleQueue::Finished;

}

void QueueOarTest::testParseQueueLine()
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

QTEST_MAIN(QueueOarTest)

#include "oartest.moc"
