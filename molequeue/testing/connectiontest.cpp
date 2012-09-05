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

#include "testing/connectiontest.h"
#include "testing/testserver.h"
#include "program.h"
#include "jobmanager.h"

bool QueueDummy::submitJob(const MoleQueue::Job)
{
  return true;
}

void ConnectionTest::initTestCase()
{
}

void ConnectionTest::cleanupTestCase()
{

}

void ConnectionTest::init()
{
  m_connectionName = TestServer::getRandomSocketName();
  m_server = new MoleQueue::Server (this, m_connectionName);
  m_server->start();
  m_client = createClient();

  // Let the event loop run a bit to handle the connections
  qApp->processEvents(QEventLoop::AllEvents, 1000);
}

void ConnectionTest::cleanup()
{
  delete m_client;

  m_server->stop();
  delete m_server;
}

void ConnectionTest::testRequestQueueList()
{
  MoleQueue::QueueListType testQueues;

  QString queueName = "Some big ol' cluster";
  QStringList progNames;
  progNames << "Quantum Tater" << "Crystal Math" << "Nebulous Nucleus";
  testQueues[queueName] = progNames;


  queueName = "Puny local queue";
  progNames.clear();
  progNames << "SpectroCrunch" << "FastFocker" << "SpeedSlater";
  testQueues[queueName] = progNames;

  // Create phony queues
  MoleQueue::QueueManager* qmanager = m_server->queueManager();

  foreach(QString qn, testQueues.keys()) {
    MoleQueue::Queue *queue = qmanager->addQueue(qn, "Local");

    foreach(QString progName, testQueues.value(qn)) {
      MoleQueue::Program *prog = new MoleQueue::Program (NULL);
      prog->setName(progName);
      queue->addProgram(prog);
    }
  }

  m_client->connectToServer(m_connectionName);

  QSignalSpy spy (m_client,
                  SIGNAL(queueListUpdated(const MoleQueue::QueueListType&)));

  m_client->requestQueueListUpdate();

  QTimer timer;
  timer.setSingleShot(true);
  timer.start(1000);
  while (timer.isActive() && spy.isEmpty()) {
    qApp->processEvents(QEventLoop::AllEvents, 500);
  }

  QCOMPARE(spy.count(), 1);

  // Verify we get the same queue list back
  MoleQueue::QueueListType queueList =
      spy.first().first().value<MoleQueue::QueueListType>();

  foreach(QString qn, testQueues.keys()) {

    QVERIFY2(queueList.contains(qn), "Missing queue");

    foreach(QString pn, testQueues.value(qn)) {
      QVERIFY2(queueList.value(qn).contains(pn), "Missing program");
    }
  }
}

void ConnectionTest::testSuccessfulJobSubmission()
{
  QString qn = "fifo";
  // Setup a queue
  MoleQueue::QueueManager* qmanager = m_server->queueManager();
  qmanager->addQueue(qn, "Local");

  m_client->connectToServer(m_connectionName);

  MoleQueue::Job req = m_client->newJobRequest();
  req.setLocalWorkingDirectory("/tmp/some/path");
  req.setMoleQueueId(1);
  req.setQueueId(1439932);
  req.setQueue(qn);
  m_client->m_jobManager->setJobQueueId(req.moleQueueId(), req.queueId());

  QSignalSpy spy (m_client,
                  SIGNAL(jobSubmitted(const MoleQueue::JobRequest&, bool,
                                      const QString&)));

  m_client->submitJobRequest(req);

  QTimer timer;
  timer.setSingleShot(true);
  timer.start(10000);
  while (timer.isActive() && spy.isEmpty()) {
    qApp->processEvents(QEventLoop::AllEvents, 500);
  }

  QCOMPARE(spy.count(), 1);


}

void ConnectionTest::testFailedSubmission()
{

  m_client->connectToServer(m_connectionName);

  MoleQueue::Job req = m_client->newJobRequest();
  req.setLocalWorkingDirectory("/tmp/some/path");
  req.setMoleQueueId(1);
  req.setQueueId(1439932);
  req.setQueue("missingQueue");
  m_client->m_jobManager->setJobQueueId(req.moleQueueId(), req.queueId());

  QSignalSpy spy (m_client,
                  SIGNAL(jobSubmitted(const MoleQueue::JobRequest&, bool,
                                      const QString&)));

  m_client->submitJobRequest(req);

  QTimer timer;
  timer.setSingleShot(true);
  timer.start(10000);
  while (timer.isActive() && spy.isEmpty()) {
    qApp->processEvents(QEventLoop::AllEvents, 500);
  }

  QCOMPARE(spy.count(), 1);

  bool success = spy.first().at(1).value<bool>();
  QString errorString  = spy.first().at(2).value<QString>();

  QCOMPARE(success, false);
  QCOMPARE(errorString, QString("Unknown queue: missingQueue"));
}

void ConnectionTest::testSuccessfulJobCancellation()
{
  QString qn = "fifo";
  // Setup a queue
  MoleQueue::QueueManager* qmanager = m_server->queueManager();
  qmanager->addQueue("fifo", QString("sge"), true);

  m_client->connectToServer(m_connectionName);

  MoleQueue::Job req = m_client->newJobRequest();
  req.setLocalWorkingDirectory("/tmp/some/path");
  req.setMoleQueueId(1);
  req.setQueueId(1439932);
  req.setQueue(qn);
  m_client->m_jobManager->setJobQueueId(req.moleQueueId(), req.queueId());

  QSignalSpy jobSubmittedSpy (m_client,
                              SIGNAL(jobSubmitted(const MoleQueue::JobRequest&,
                                                  bool, const QString&)));

  m_client->submitJobRequest(req);

  QTimer timer;
  timer.setSingleShot(true);
  timer.start(10000);
  while (timer.isActive() && jobSubmittedSpy.isEmpty()) {
    qApp->processEvents(QEventLoop::AllEvents, 500);
  }

  QCOMPARE(jobSubmittedSpy.count(), 1);

  QSignalSpy jobCancelledSpy (m_client,
                              SIGNAL(jobCanceled(const MoleQueue::JobRequest&,
                                                 bool, const QString&)));

  m_client->cancelJob(req);

  timer.start(10000);
  while (timer.isActive() && jobCancelledSpy.isEmpty()) {
    qApp->processEvents(QEventLoop::AllEvents, 500);
  }

  QCOMPARE(jobCancelledSpy.count(), 1);

  bool success = jobCancelledSpy.first().at(1).value<bool>();
  QString errorString  = jobCancelledSpy.first().at(2).value<QString>();

  QCOMPARE(success, true);
  QCOMPARE(errorString, QString());

}

void ConnectionTest::testJobStateChangeNotification()
{
  QString queueName = "fifo";
  QString programName = "program.exe";
  // Setup a queue
  MoleQueue::QueueManager* qmanager = m_server->queueManager();
  MoleQueue::Queue *queue = qmanager->addQueue(queueName, QString("Local"));
  MoleQueue::Program *program = new MoleQueue::Program(queue);
  program->setName(programName);
  queue->addProgram(program);

  MoleQueue::Job req = m_client->newJobRequest();
  req.setLocalWorkingDirectory("/tmp/some/path");
  req.setQueue(queueName);
  req.setProgram(programName);

  QSignalSpy jobSubmittedSpy (m_client,
                              SIGNAL(jobSubmitted(
                                       const MoleQueue::JobRequest&,
                                       bool, const QString&)));

  QSignalSpy jobStateChangedSpy (m_client,
                                 SIGNAL(jobStateChanged(
                                          const MoleQueue::JobRequest&,
                                          MoleQueue::JobState,
                                          MoleQueue::JobState)));

  m_client->connectToServer(m_connectionName);
  m_client->submitJobRequest(req);

  QTimer timer;
  timer.setSingleShot(true);
  timer.start(1000);
  while (timer.isActive() && jobSubmittedSpy.isEmpty()) {
    qApp->processEvents(QEventLoop::AllEvents, 500);
  }

  QCOMPARE(jobSubmittedSpy.count(), 1);

  bool success = jobSubmittedSpy.first().at(1).value<bool>();
  QVERIFY(success);

  MoleQueue::JobRequest job =
      jobSubmittedSpy.first().at(0).value<MoleQueue::JobRequest>();

  MoleQueue::JobManager *jobManager = m_server->jobManager();

  jobManager->setJobState(job.moleQueueId(), MoleQueue::Killed);

  timer.start(1000);
  while (timer.isActive()) {
    qApp->processEvents(QEventLoop::AllEvents, 500);
  }

  QCOMPARE(jobStateChangedSpy.count(), 3);

  MoleQueue::JobState state =
      jobStateChangedSpy.last().at(2).value<MoleQueue::JobState>();

  QCOMPARE(state, MoleQueue::Killed);
}
