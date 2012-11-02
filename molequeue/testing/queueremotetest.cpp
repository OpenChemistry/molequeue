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

#include "dummyqueueremote.h"

#include "dummyqueuemanager.h"
#include "dummyserver.h"
#include "filesystemtools.h"
#include "job.h"
#include "jobmanager.h"
#include "program.h"

#include <QtCore/QDir>
#include <QtCore/QFile>

using namespace MoleQueue;

class QueueRemoteTest : public QObject
{
  Q_OBJECT
private:

  DummyServer m_server;
  DummyQueueRemote *m_queue;

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
  void testSubmitJob();
  void testKillJob();
  void testSubmissionPipeline();
  void testFinalizePipeline();
  void testKillPipeline();
  void testQueueUpdate();
  void testReplaceLaunchScriptKeywords();
};

void QueueRemoteTest::initTestCase()
{
  m_queue = qobject_cast<DummyQueueRemote*>
      (m_server.queueManager()->addQueue("Dummy", "Dummy", true));
  m_queue->setWorkingDirectoryBase("/fake/remote/path");

  Program *program = new Program(m_queue);
  program->setName("DummyProgram");
  program->setExecutable("");
  program->setUseExecutablePath(false);
  program->setArguments("");
  program->setInputFilename("input.in");
  program->setOutputFilename("output.out");
  program->setLaunchSyntax(Program::REDIRECT);
  m_queue->addProgram(program);
}

void QueueRemoteTest::cleanupTestCase()
{
  FileSystemTools::recursiveRemoveDirectory(m_server.workingDirectoryBase());
}

void QueueRemoteTest::init()
{
}

void QueueRemoteTest::cleanup()
{
}

void QueueRemoteTest::sanityCheck()
{
  QCOMPARE(m_queue->typeName(), QString("Dummy"));
  QString testString = "some.host.somewhere";
  m_queue->setHostName(testString);
  QCOMPARE(m_queue->hostName(), testString);
  testString = "aUser";
  m_queue->setUserName(testString);
  QCOMPARE(m_queue->userName(), testString);
  m_queue->setSshPort(6887);
  QCOMPARE(m_queue->sshPort(), 6887);
  testString = "/some/path";
  m_queue->setWorkingDirectoryBase(testString);
  QCOMPARE(m_queue->workingDirectoryBase(), testString);
  testString = "subComm";
  m_queue->setSubmissionCommand(testString);
  QCOMPARE(m_queue->submissionCommand(), testString);
  testString = "reqComm";
  m_queue->setRequestQueueCommand(testString);
  QCOMPARE(m_queue->requestQueueCommand(), testString);
  testString = "killComm";
  m_queue->setKillCommand(testString);
  QCOMPARE(m_queue->killCommand(), testString);
}

void QueueRemoteTest::testSubmitJob()
{
  // valid job
  Job job = m_server.jobManager()->newJob();
  QVERIFY(m_queue->submitJob(job));

  // invalid job
  QVERIFY(!m_queue->submitJob(Job()));

  // verify queue state
  QCOMPARE(m_queue->m_pendingSubmission.size(), 1);
  QCOMPARE(m_queue->m_pendingSubmission.first(), job.moleQueueId());
}

void QueueRemoteTest::testKillJob()
{
  // Invalid job
  m_queue->killJob(Job());

  // unknown job
  Job job = m_server.jobManager()->newJob();
  m_queue->killJob(job);
  QCOMPARE(job.jobState(), Killed);

  // pending job (from testSubmitJob)
  job = Job(m_server.jobManager(), m_queue->m_pendingSubmission.first());
  m_queue->killJob(job);
  QCOMPARE(job.jobState(), Killed);
  QCOMPARE(m_queue->m_pendingSubmission.size(), 0);

  // "Running" job:
  job = m_server.jobManager()->newJob();
  job.setQueue("Dummy");
  job.setQueueId(999);
  m_queue->m_jobs.insert(job.queueId(), job.moleQueueId());
  m_queue->killJob(job); // Won't actually kill job, but starts the killPipeline
  QCOMPARE(m_queue->m_jobs.size(), 0);
}

void QueueRemoteTest::testSubmissionPipeline()
{
  ///////////////////////
  // submitPendingJobs //
  ///////////////////////

  // empty pending queue
  QCOMPARE(m_queue->m_pendingSubmission.size(), 0);
  m_queue->submitPendingJobs();
  QCOMPARE(m_queue->m_pendingSubmission.size(), 0);

  // Create and submit "actual fake" job:
  Job job = m_server.jobManager()->newJob();
  job.setQueue("Dummy");
  job.setProgram("DummyProgram");
  job.setDescription("DummyJob");
  job.setInputFile(FileSpecification("file.ext", "do stuff, return answers."));
  job.setOutputDirectory(job.localWorkingDirectory() + "/../output");
  job.setCleanRemoteFiles(true);
  job.setCleanLocalWorkingDirectory(true);
  m_queue->submitJob(job);

  QCOMPARE(m_queue->m_pendingSubmission.size(), 1);
  m_queue->submitPendingJobs(); // calls beginJobSubmission
  QCOMPARE(m_queue->m_pendingSubmission.size(), 0);

  ////////////////////////
  // beginJobSubmission // (calls writeInputFiles, copyInputFilesToHost)
  ////////////////////////

  /////////////////////
  // writeInputFiles //
  /////////////////////

  // Check that input files were written:
  Program *program = m_queue->lookupProgram(job.program());
  QVERIFY(program != NULL);
  QString inputFileName = job.localWorkingDirectory() + "/"
      + program->inputFilename();
  QVERIFY(QFile::exists(inputFileName));
  QFile inputFile(inputFileName);
  QVERIFY(inputFile.open(QFile::ReadOnly | QFile::Text));
  QCOMPARE(QString(inputFile.readAll()), job.inputFile().contents());

  QString launchScriptFileName = job.localWorkingDirectory() + "/"
      + m_queue->launchScriptName();
  QVERIFY(QFile::exists(launchScriptFileName));
  QFile launchScriptFile(launchScriptFileName);
  QVERIFY(launchScriptFile.open(QFile::ReadOnly | QFile::Text));
  QCOMPARE(QString(launchScriptFile.readAll()),
           QString("Run job 4!!\n"));

  //////////////////////////
  // copyInputFilesToHost //
  //////////////////////////

  // validate the ssh command
  DummySshCommand *ssh = m_queue->getDummySshCommand();
  QCOMPARE(ssh->getDummyCommand(), QString("scp"));
  // Verify the local path manually with a regex, since the absolute path is
  // platform dependent
  QStringList dummyArgs = ssh->getDummyArgs();
  QVERIFY(QRegExp("^.+MoleQueue-dummyServer/+jobs/+4$").exactMatch(
            dummyArgs.at(6)));
  dummyArgs.removeAt(6);
  QCOMPARE(dummyArgs, QStringList()
           << "-q"
           << "-S" << "ssh"
           << "-P" << "6887"
           << "-r"
           << "aUser@some.host.somewhere:/some/path/4"
           );
  QCOMPARE(ssh->data().value<Job>(), job);

  // Fake the process output. Pretend that the remote working dir hasn't been
  // created yet.
  ssh->setDummyExitCode(1);
  ssh->setDummyOutput("No such file or directory");
  ssh->emitDummyRequestComplete(); // triggers inputFilesCopied

  //////////////////////
  // inputFilesCopied // // Should detect that the parent dir doesn't exist
  ////////////////////// // and call create remote directory

  ///////////////////////////
  // createRemoteDirectory //
  ///////////////////////////

  // Grab the dummy ssh command from the queue and validate its contents
  ssh = m_queue->getDummySshCommand();
  QCOMPARE(ssh->getDummyCommand(), QString("ssh"));
  QCOMPARE(ssh->getDummyArgs(), QStringList()
           << "-q"
           << "-p" << "6887"
           << "aUser@some.host.somewhere"
           << "mkdir -p /some/path"
           );
  QCOMPARE(ssh->data().value<Job>(), job);

  // Fake the process output
  ssh->setDummyExitCode(0);
  ssh->emitDummyRequestComplete(); // triggers remoteDirectoryCreated

  ////////////////////////////
  // remoteDirectoryCreated // // calls copyInputFilesToHost
  ////////////////////////////

  //////////////////////////
  // copyInputFilesToHost //
  //////////////////////////

  // validate the ssh command
  ssh = m_queue->getDummySshCommand();
  QCOMPARE(ssh->getDummyCommand(), QString("scp"));
  // Verify the local path manually with a regex, since the absolute path is
  // platform dependent
  dummyArgs = ssh->getDummyArgs();
  QVERIFY(QRegExp("^.+MoleQueue-dummyServer/+jobs/+4$").exactMatch(
            dummyArgs.at(6)));
  dummyArgs.removeAt(6);
  QCOMPARE(dummyArgs, QStringList()
           << "-q"
           << "-S" << "ssh"
           << "-P" << "6887"
           << "-r"
           << "aUser@some.host.somewhere:/some/path/4"
           );
  QCOMPARE(ssh->data().value<Job>(), job);

  // Fake the process output
  ssh->setDummyExitCode(0);
  ssh->emitDummyRequestComplete(); // triggers inputFilesCopied

  //////////////////////
  // inputFilesCopied // // calls submitJobToRemoteQueue
  //////////////////////

  ////////////////////////////
  // submitJobToRemoteQueue //
  ////////////////////////////

  // validate the ssh command
  ssh = m_queue->getDummySshCommand();
  QCOMPARE(ssh->getDummyCommand(), QString("ssh"));
  QCOMPARE(ssh->getDummyArgs(), QStringList()
           << "-q"
           << "-p" << "6887"
           << "aUser@some.host.somewhere"
           << "cd /some/path/4 && subComm launcher.dummy"
           );
  QCOMPARE(ssh->data().value<Job>(), job);

  // Fake the process output
  ssh->setDummyExitCode(0);
  ssh->emitDummyRequestComplete(); // triggers jobSubmittedToRemoteQueue

  ///////////////////////////////
  // jobSubmittedToRemoteQueue //
  ///////////////////////////////

  // Check that the job has been updated
  QCOMPARE(m_queue->m_jobs.keys().size(), 1);
  QCOMPARE(job.queueId(), static_cast<IdType>(12));
  QCOMPARE(job.jobState(), Submitted);
}

void QueueRemoteTest::testFinalizePipeline()
{
  // Kick off the finalize pipeline:
  QCOMPARE(m_queue->m_jobs.keys().size(), 1);
  Job job = m_server.jobManager()->lookupJobByMoleQueueId(
        m_queue->m_jobs.values().first());
  m_queue->beginFinalizeJob(m_queue->m_jobs.keys().first());

  //////////////////////
  // beginFinalizeJob // (calls finalizeJobCopyFromServer)
  //////////////////////

  QCOMPARE(m_queue->m_jobs.size(), 0);

  ///////////////////////////////
  // finalizeJobCopyFromServer //
  ///////////////////////////////

  // validate the ssh command
  DummySshCommand *ssh = m_queue->getDummySshCommand();
  QCOMPARE(ssh->getDummyCommand(), QString("scp"));
  // Verify the local path manually with a regex, since the absolute path is
  // platform dependent
  QStringList dummyArgs = ssh->getDummyArgs();
  QVERIFY(QRegExp("^.+MoleQueue-dummyServer/+jobs/+4/+\\.\\.$").exactMatch(
            dummyArgs.at(7)));
  dummyArgs.removeAt(7);
  QCOMPARE(dummyArgs, QStringList()
           << "-q"
           << "-S" << "ssh"
           << "-P" << "6887"
           << "-r"
           << "aUser@some.host.somewhere:/some/path/4"
           );
  QCOMPARE(ssh->data().value<Job>(), job);

  // Fake the process output
  ssh->setDummyExitCode(0);
  ssh->emitDummyRequestComplete(); // triggers finalizeJobOutputCopiedFromServer

  ///////////////////////////////////////
  // finalizeJobOutputCopiedFromServer // (calls
  ///////////////////////////////////////   finalizeJobCopyToCustomDestination)

  ////////////////////////////////////////
  // finalizeJobCopyToCustomDestination // (calls recursiveCopyDirectory
  ////////////////////////////////////////   and finalizeJobCleanup)

  ////////////////////////////
  // recursiveCopyDirectory //
  ////////////////////////////

  QCOMPARE(QDir(job.localWorkingDirectory()).entryList(),
           QDir(job.outputDirectory()).entryList());

  ////////////////////////
  // finalizeJobCleanup // (calls cleanLocalDirectory
  ////////////////////////   and cleanRemoteDirectory)

  QCOMPARE(job.jobState(), Finished);

  /////////////////////////
  // cleanLocalDirectory //
  /////////////////////////

  QCOMPARE(QDir(job.localWorkingDirectory()).entryList(), QStringList());

  //////////////////////////
  // cleanRemoteDirectory //
  //////////////////////////

  // validate the ssh command
  ssh = m_queue->getDummySshCommand();
  QCOMPARE(ssh->getDummyCommand(), QString("ssh"));
  QCOMPARE(ssh->getDummyArgs(), QStringList()
           << "-q"
           << "-p" << "6887"
           << "aUser@some.host.somewhere"
           << "rm -rf /some/path/4"
           );
  QCOMPARE(ssh->data().value<Job>(), job);

  // Fake the process output
  ssh->setDummyExitCode(0);
  ssh->emitDummyRequestComplete(); // triggers remoteDirectoryCleaned

  ////////////////////////////
  // remoteDirectoryCleaned //
  ////////////////////////////

  // no state changes to check

}

void QueueRemoteTest::testKillPipeline()
{
  // Fake a submitted job
  Job job = m_server.jobManager()->newJob();
  job.setQueue("Dummy");
  job.setQueueId(988);
  m_queue->m_jobs.insert(job.queueId(), job.moleQueueId());

  // kill the job
  m_queue->killJob(job); // calls beginKillJob

  //////////////////
  // beginKillJob //
  //////////////////

  // validate the ssh command
  DummySshCommand *ssh = m_queue->getDummySshCommand();
  QCOMPARE(ssh->getDummyCommand(), QString("ssh"));
  QCOMPARE(ssh->getDummyArgs(), QStringList()
           << "-q"
           << "-p" << "6887"
           << "aUser@some.host.somewhere"
           << "killComm 988"
           );
  QCOMPARE(ssh->data().value<Job>(), job);

  // Fake the process output
  ssh->setDummyExitCode(0);
  ssh->emitDummyRequestComplete(); // triggers endKillJob

  ////////////////
  // endKillJob //
  ////////////////

  QCOMPARE(job.jobState(), Killed);
}

void QueueRemoteTest::testQueueUpdate()
{
  QCOMPARE(m_queue->m_jobs.size(), 0);
  QString output;
  QList<Job> jobs;
  for (int state = static_cast<int>(None);
       state < static_cast<int>(Error); ++state) {
    // Add jobs with queueids
    Job job = m_server.jobManager()->newJob();
    job.setQueue("Dummy");
    job.setQueueId(static_cast<IdType>(state));
    jobs.append(job);

    // add to queue
    m_queue->m_jobs.insert(job.queueId(), job.moleQueueId());

    // Create line of fake queue output
    output += QString("%1 %2\n").arg(job.queueId())
        .arg(jobStateToString(static_cast<JobState>(state)));
  }

  m_queue->requestQueueUpdate();

  ////////////////////////
  // requestQueueUpdate //
  ////////////////////////

  // validate the ssh command
  DummySshCommand *ssh = m_queue->getDummySshCommand();
  QCOMPARE(ssh->getDummyCommand(), QString("ssh"));
  QCOMPARE(ssh->getDummyArgs(), QStringList()
           << "-q"
           << "-p" << "6887"
           << "aUser@some.host.somewhere"
           << "reqComm 0 1 2 3 4 5 6 7 8 "
           );

  // Fake the process output
  ssh->setDummyExitCode(0);
  ssh->setDummyOutput(output);
  ssh->emitDummyRequestComplete(); // triggers handleQueueUpdate

  ///////////////////////
  // handleQueueUpdate //
  ///////////////////////

  foreach (const Job &job, jobs) {
    QCOMPARE(job.jobState(),
             static_cast<JobState>(job.queueId()));
  }

}

void QueueRemoteTest::testReplaceLaunchScriptKeywords()
{
  // $$maxWallTime$$
  QStringList list;
  m_queue->setDefaultMaxWallTime(1440);
  list << "$$maxWallTime$$ at start"
       << "At end $$maxWallTime$$"
       << "In middle $$maxWallTime$$ of line";
  QString script = list.join("\n");

  Job job = m_server.jobManager()->newJob();
  job.setMaxWallTime(-1);
  m_queue->replaceLaunchScriptKeywords(script, job);
  QCOMPARE(script,
           QString("24:00:00 at start\nAt end 24:00:00\n"
                   "In middle 24:00:00 of line\n"));

  // $$$maxWallTime$$$
  list.clear();
  list << "Test first line"
       << "$$$maxWallTime$$$ at start"
       << "Test third line"
       << "At end $$$maxWallTime$$$"
       << "Test fifth line"
       << "In middle $$$maxWallTime$$$ of line"
       << "Test sixth line"
       << "Safe maxWallTime=$$maxWallTime$$";
  script = list.join("\n");

  m_queue->replaceLaunchScriptKeywords(script, job);
  QCOMPARE(script,
           QString("Test first line\nTest third line\nTest fifth line\n"
                   "Test sixth line\nSafe maxWallTime=24:00:00\n"));
}

QTEST_MAIN(QueueRemoteTest)

#include "queueremotetest.moc"
