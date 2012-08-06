/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "remote.h"

#include "../job.h"
#include "../jobmanager.h"
#include "../logentry.h"
#include "../logger.h"
#include "../program.h"
#include "../remotequeuewidget.h"
#include "../server.h"
#include "../sshcommand.h"

#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include <QtGui>

namespace MoleQueue {

QueueRemote::QueueRemote(const QString &queueName, QueueManager *parentObject)
  : Queue(queueName, parentObject),
    m_sshPort(22),
    m_isCheckingQueue(false),
    m_checkForPendingJobsTimerId(-1),
    m_defaultMaxWallTime(DEFAULT_MAX_WALLTIME)
{
  // Check remote queue every 60 seconds
  m_checkQueueTimerId = startTimer(60000);

  // Check for jobs to submit every 5 seconds
  m_checkForPendingJobsTimerId = startTimer(5000);

  // Always allow m_requestQueueCommand to return 0
  m_allowedQueueRequestExitCodes.append(0);
}

QueueRemote::~QueueRemote()
{
}

void QueueRemote::readSettings(QSettings &settings)
{
  Queue::readSettings(settings);

  m_workingDirectoryBase = settings.value("workingDirectoryBase").toString();
  m_submissionCommand = settings.value("submissionCommand").toString();
  m_requestQueueCommand = settings.value("requestQueueCommand").toString();
  m_killCommand = settings.value("killCommand").toString();
  m_hostName = settings.value("hostName").toString();
  m_userName = settings.value("userName").toString();
  m_sshPort  = settings.value("sshPort").toInt();
}

void QueueRemote::writeSettings(QSettings &settings) const
{
  Queue::writeSettings(settings);

  settings.setValue("workingDirectoryBase", m_workingDirectoryBase);
  settings.setValue("submissionCommand", m_submissionCommand);
  settings.setValue("requestQueueCommand", m_requestQueueCommand);
  settings.setValue("killCommand", m_killCommand);
  settings.setValue("hostName", m_hostName);
  settings.setValue("userName", m_userName);
  settings.setValue("sshPort",  m_sshPort);
}

void QueueRemote::exportConfiguration(QSettings &exporter,
                                      bool includePrograms) const
{
  Queue::exportConfiguration(exporter, includePrograms);

  exporter.setValue("submissionCommand", m_submissionCommand);
  exporter.setValue("requestQueueCommand", m_requestQueueCommand);
  exporter.setValue("killCommand", m_killCommand);
  exporter.setValue("hostName", m_hostName);
  exporter.setValue("sshPort",  m_sshPort);
}

void QueueRemote::importConfiguration(QSettings &importer,
                                      bool includePrograms)
{
  Queue::importConfiguration(importer, includePrograms);

  m_submissionCommand = importer.value("submissionCommand").toString();
  m_requestQueueCommand = importer.value("requestQueueCommand").toString();
  m_killCommand = importer.value("killCommand").toString();
  m_hostName = importer.value("hostName").toString();
  m_sshPort  = importer.value("sshPort").toInt();
}

QWidget* QueueRemote::settingsWidget()
{
  RemoteQueueWidget *widget = new RemoteQueueWidget (this);
  return widget;
}

void QueueRemote::replaceLaunchScriptKeywords(QString &launchScript,
                                              const Job &job)
{
  int wallTime = job.maxWallTime();
  if (launchScript.contains("$$$maxWallTime$$$")) {
    // If a valid walltime is set, replace all occurances with the appropriate
    // string:
    if (wallTime > 0) {
      int hours = wallTime / 60;
      int minutes = wallTime % 60;
      launchScript.replace("$$$maxWallTime$$$",
                           QString("%1:%2:00")
                           .arg(hours, 2, 10, QChar('0'))
                           .arg(minutes, 2, 10, QChar('0')));
    }
    // Otherwise, erase all lines containing the keyword
    else {
      QRegExp expr("\\n[^\\n]*\\${3,3}maxWallTime\\${3,3}[^\\n]*\\n");
      launchScript.replace(expr, "\n");
    }
  }

  if (launchScript.contains("$$maxWallTime$$")) {
    if (wallTime <= 0)
      wallTime = defaultMaxWallTime();
    int hours = wallTime / 60;
    int minutes = wallTime % 60;
    launchScript.replace("$$maxWallTime$$",
                         QString("%1:%2:00")
                         .arg(hours, 2, 10, QChar('0'))
                         .arg(minutes, 2, 10, QChar('0')));
  }

  Queue::replaceLaunchScriptKeywords(launchScript, job);
}

bool QueueRemote::submitJob(Job job)
{
  if (job.isValid()) {
    m_pendingSubmission.append(job.moleQueueId());
    job.setJobState(MoleQueue::Accepted);
    return true;
  }
  return false;
}

void QueueRemote::killJob(Job job)
{
  if (!job.isValid())
    return;

  int pendingIndex = m_pendingSubmission.indexOf(job.moleQueueId());
  if (pendingIndex >= 0) {
    m_pendingSubmission.removeAt(pendingIndex);
    job.setJobState(MoleQueue::Killed);
    return;
  }

  if (job.queue() == m_name && job.queueId() != InvalidId &&
      m_jobs.value(job.queueId()) == job.moleQueueId()) {
    m_jobs.remove(job.queueId());
    beginKillJob(job);
    return;
  }

  Logger::logWarning(tr("Queue '%1' requested to kill unknown job that belongs "
                        "to queue '%2', queue id '%3'.").arg(m_name)
                     .arg(job.queue())
                     .arg(job.queueId() != InvalidId
                          ? QString::number(job.queueId())
                          : QString("(Invalid)")), job.moleQueueId());
  job.setJobState(MoleQueue::Killed);
}

void QueueRemote::submitPendingJobs()
{
  if (m_pendingSubmission.isEmpty())
    return;

  // lookup job manager:
  JobManager *jobManager = NULL;
  if (m_server)
    jobManager = m_server->jobManager();

  if (!jobManager) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Cannot locate server JobManager!"));
    return;
  }

  foreach (const IdType moleQueueId, m_pendingSubmission) {
    Job job = jobManager->lookupJobByMoleQueueId(moleQueueId);
    // Kick off the submission process...
    beginJobSubmission(job);
  }

  m_pendingSubmission.clear();
}

void QueueRemote::beginJobSubmission(Job job)
{
  if (!writeInputFiles(job))
    return;

  createRemoteDirectory(job);
}

void QueueRemote::createRemoteDirectory(Job job)
{
  // Note that this is just the working directory base -- the job folder is
  // created by scp.
  QString remoteDir = QString("%1").arg(m_workingDirectoryBase);

  SshConnection *conn = newSshConnection();
  conn->setData(QVariant::fromValue(job));
  connect(conn, SIGNAL(requestComplete()), this, SLOT(remoteDirectoryCreated()));

  if (!conn->execute(QString("mkdir -p %1").arg(remoteDir))) {
    Logger::logError(tr("Could not initialize ssh resources: user= '%1'\nhost ="
                        " '%2' port = '%3'")
                     .arg(conn->userName()).arg(conn->hostName())
                     .arg(conn->portNumber()), job.moleQueueId());
    job.setJobState(MoleQueue::ErrorState);
    conn->deleteLater();
    return;
  }
}

void QueueRemote::remoteDirectoryCreated()
{
  SshConnection *conn = qobject_cast<SshConnection*>(sender());
  if (!conn) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not an SshConnection!"));
    return;
  }
  conn->deleteLater();

  Job job = conn->data().value<Job>();

  if (!job.isValid()) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender does not have an associated job!"));
    return;
  }

  if (conn->exitCode() != 0) {
    Logger::logWarning(tr("Cannot create remote directory '%1@%2:%3'. Retrying "
                          "soon.\nExit code (%4) %5")
                       .arg(conn->userName()).arg(conn->hostName())
                       .arg(m_workingDirectoryBase).arg(conn->exitCode())
                       .arg(conn->output()), job.moleQueueId());
    // Retry submission:
    m_pendingSubmission.append(job.moleQueueId());
    job.setJobState(MoleQueue::ErrorState);
    return;
  }

  copyInputFilesToHost(job);
}

void QueueRemote::copyInputFilesToHost(Job job)
{
  QString localDir = job.localWorkingDirectory();
  QString remoteDir =
      QString("%1/").arg(m_workingDirectoryBase);

  SshConnection *conn = newSshConnection();
  conn->setData(QVariant::fromValue(job));
  connect(conn, SIGNAL(requestComplete()), this, SLOT(inputFilesCopied()));

  if (!conn->copyDirTo(localDir, remoteDir)) {
    Logger::logError(tr("Could not initialize ssh resources: user= '%1'\nhost ="
                        " '%2' port = '%3'")
                     .arg(conn->userName()).arg(conn->hostName())
                     .arg(conn->portNumber()), job.moleQueueId());
    job.setJobState(MoleQueue::ErrorState);
    conn->deleteLater();
    return;
  }
}

void QueueRemote::inputFilesCopied()
{
  SshConnection *conn = qobject_cast<SshConnection*>(sender());
  if (!conn) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not an SshConnection!"));
    return;
  }
  conn->deleteLater();

  Job job = conn->data().value<Job>();

  if (!job.isValid()) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender does not have an associated job!"));
    return;
  }

  if (conn->exitCode() != 0) {
    Logger::logWarning(tr("Error while copying input files to remote host:\n"
                          "'%1' --> '%2/'\nExit code (%3) %4\n\nRetrying soon.")
                       .arg(job.localWorkingDirectory())
                       .arg(m_workingDirectoryBase)
                       .arg(conn->exitCode()).arg(conn->output()),
                       job.moleQueueId());
    // Retry submission:
    m_pendingSubmission.append(job.moleQueueId());
    job.setJobState(MoleQueue::ErrorState);
    return;
  }

  submitJobToRemoteQueue(job);
}

void QueueRemote::submitJobToRemoteQueue(Job job)
{
  const QString command = QString("cd %1/%2 && %3 %4")
      .arg(m_workingDirectoryBase)
      .arg(job.moleQueueId())
      .arg(m_submissionCommand)
      .arg(m_launchScriptName);

  SshConnection *conn = newSshConnection();
  conn->setData(QVariant::fromValue(job));
  connect(conn, SIGNAL(requestComplete()),
          this, SLOT(jobSubmittedToRemoteQueue()));

  if (!conn->execute(command)) {
    Logger::logError(tr("Could not initialize ssh resources: user= '%1'\nhost ="
                        " '%2' port = '%3'")
                     .arg(conn->userName()).arg(conn->hostName())
                     .arg(conn->portNumber()), job.moleQueueId());
    job.setJobState(MoleQueue::ErrorState);
    conn->deleteLater();
    return;
  }
}

void QueueRemote::jobSubmittedToRemoteQueue()
{
  SshConnection *conn = qobject_cast<SshConnection*>(sender());
  if (!conn) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not an SshConnection!"));
    return;
  }
  conn->deleteLater();

  IdType queueId;
  parseQueueId(conn->output(), &queueId);
  Job job = conn->data().value<Job>();

  if (!job.isValid()) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender does not have an associated job!"));
    return;
  }

  if (conn->exitCode() != 0) {
    Logger::logWarning(tr("Could not submit job to remote queue on %1@%2:%3\n"
                          "%4 %5/%6/%7\nExit code (%8) %9\n\nRetrying soon.")
                       .arg(conn->userName()).arg(conn->hostName())
                       .arg(conn->portNumber()).arg(m_submissionCommand)
                       .arg(m_workingDirectoryBase).arg(job.moleQueueId())
                       .arg(m_launchScriptName).arg(conn->exitCode())
                       .arg(conn->output()), job.moleQueueId());
    // Retry submission:
    m_pendingSubmission.append(job.moleQueueId());
    job.setJobState(MoleQueue::ErrorState);
    return;
  }

  job.setJobState(MoleQueue::Submitted);
  job.setQueueId(queueId);
  m_jobs.insert(queueId, job.moleQueueId());
}

void QueueRemote::requestQueueUpdate()
{
  if (m_isCheckingQueue)
    return;

  m_isCheckingQueue = true;

  const QString command = generateQueueRequestCommand();

  SshConnection *conn = newSshConnection();
  connect(conn, SIGNAL(requestComplete()),
          this, SLOT(handleQueueUpdate()));

  if (!conn->execute(command)) {
    Logger::logError(tr("Could not initialize ssh resources: user= '%1'\nhost ="
                        " '%2' port = '%3'")
                     .arg(conn->userName()).arg(conn->hostName())
                     .arg(conn->portNumber()));
    conn->deleteLater();
    return;
  }
}

void QueueRemote::handleQueueUpdate()
{
  SshConnection *conn = qobject_cast<SshConnection*>(sender());
  if (!conn) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not an SshConnection!"));
    m_isCheckingQueue = false;
    return;
  }
  conn->deleteLater();

  if (!m_allowedQueueRequestExitCodes.contains(conn->exitCode())) {
    Logger::logWarning(tr("Error requesting queue data (%1 -u %2) on remote "
                          "host %3@%4:%5. Exit code (%6) %7")
                       .arg(m_requestQueueCommand)
                       .arg(m_userName).arg(conn->userName())
                       .arg(conn->hostName()).arg(conn->portNumber())
                       .arg(conn->exitCode()).arg(conn->output()));
    m_isCheckingQueue = false;
    return;
  }

  QStringList output = conn->output().split("\n", QString::SkipEmptyParts);

  // Get list of submitted queue ids so that we detect when jobs have left
  // the queue.
  QList<IdType> queueIds = m_jobs.keys();

  MoleQueue::JobState state;
  foreach (QString line, output) {
    IdType queueId;
    if (parseQueueLine(line, &queueId, &state)) {
      IdType moleQueueId = m_jobs.value(queueId, InvalidId);
      if (moleQueueId != InvalidId) {
        queueIds.removeOne(queueId);
        // Get pointer to jobmanager to lookup job
        if (!m_server) {
          Logger::logError(tr("Queue '%1' cannot locate Server instance!")
                           .arg(m_name), moleQueueId);
          m_isCheckingQueue = false;
          return;
        }
        Job job = m_server->jobManager()->lookupJobByMoleQueueId(moleQueueId);
        if (!job.isValid()) {
          Logger::logError(tr("Queue '%1' Cannot update invalid Job reference!")
                           .arg(m_name), moleQueueId);
          continue;
        }
        job.setJobState(state);
      }
    }
  }

  // Now copy back any jobs that have left the queue
  foreach (IdType queueId, queueIds)
    beginFinalizeJob(queueId);

  m_isCheckingQueue = false;
}

void QueueRemote::beginFinalizeJob(IdType queueId)
{
  IdType moleQueueId = m_jobs.value(queueId, InvalidId);
  if (moleQueueId == InvalidId)
    return;

  m_jobs.remove(queueId);

  // Lookup job
  if (!m_server)
    return;
  Job job = m_server->jobManager()->lookupJobByMoleQueueId(moleQueueId);
  if (!job.isValid())
    return;

  finalizeJobCopyFromServer(job);
}

void QueueRemote::finalizeJobCopyFromServer(Job job)
{
  if (!job.retrieveOutput() ||
      (job.cleanLocalWorkingDirectory() && job.outputDirectory().isEmpty())
      ) {
    // Jump to next step
    finalizeJobCopyToCustomDestination(job);
    return;
  }

  QString localDir = job.localWorkingDirectory() + "/..";
  QString remoteDir =
      QString("%1/%2").arg(m_workingDirectoryBase).arg(job.moleQueueId());
  SshConnection *conn = newSshConnection();
  conn->setData(QVariant::fromValue(job));
  connect(conn, SIGNAL(requestComplete()),
          this, SLOT(finalizeJobOutputCopiedFromServer()));

  if (!conn->copyDirFrom(remoteDir, localDir)) {
    Logger::logError(tr("Could not initialize ssh resources: user= '%1'\nhost ="
                        " '%2' port = '%3'")
                     .arg(conn->userName()).arg(conn->hostName())
                     .arg(conn->portNumber()), job.moleQueueId());
    job.setJobState(MoleQueue::ErrorState);
    conn->deleteLater();
    return;
  }
}

void QueueRemote::finalizeJobOutputCopiedFromServer()
{
  SshConnection *conn = qobject_cast<SshConnection*>(sender());
  if (!conn) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not an SshConnection!"));
    return;
  }
  conn->deleteLater();

  Job job = conn->data().value<Job>();

  if (!job.isValid()) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender does not have an associated job!"));
    return;
  }

  if (conn->exitCode() != 0) {
    Logger::logError(tr("Error while copying job output from remote server:\n"
                        "%1@%2:%3 --> %4\nExit code (%5) %6")
                     .arg(conn->userName()).arg(conn->hostName())
                     .arg(conn->portNumber()).arg(job.localWorkingDirectory())
                     .arg(conn->exitCode()).arg(conn->output()),
                     job.moleQueueId());
    job.setJobState(MoleQueue::ErrorState);
    return;
  }

  finalizeJobCopyToCustomDestination(job);
}

void QueueRemote::finalizeJobCopyToCustomDestination(Job job)
{
  // Skip to next step if needed
  if (job.outputDirectory().isEmpty() ||
      job.outputDirectory() == job.localWorkingDirectory()) {
    finalizeJobCleanup(job);
    return;
  }

  // The copy function will throw errors if needed.
  if (!recursiveCopyDirectory(job.localWorkingDirectory(),
                              job.outputDirectory())) {
    job.setJobState(MoleQueue::ErrorState);
    return;
  }

  finalizeJobCleanup(job);
}

void QueueRemote::finalizeJobCleanup(Job job)
{
  if (job.cleanLocalWorkingDirectory())
    cleanLocalDirectory(job);

  if (job.cleanRemoteFiles())
    cleanRemoteDirectory(job);

  job.setJobState(MoleQueue::Finished);
}

void QueueRemote::cleanRemoteDirectory(Job job)
{
  QString remoteDir = QDir::cleanPath(
        QString("%1/%2").arg(m_workingDirectoryBase).arg(job.moleQueueId()));

  // Check that the remoteDir is not just "/" due to another bug.
  if (remoteDir.simplified() == "/") {
    Logger::logError(tr("Refusing to clean remote directory %1 -- an internal "
                        "error has occurred.").arg(remoteDir),
                     job.moleQueueId());
    return;
  }

  QString command = QString ("rm -rf %1").arg(remoteDir);

  SshConnection *conn = newSshConnection();
  conn->setData(QVariant::fromValue(job));
  connect(conn, SIGNAL(requestComplete()),
          this, SLOT(remoteDirectoryCleaned()));

  if (!conn->execute(command)) {
    Logger::logError(tr("Could not initialize ssh resources: user= '%1'\nhost ="
                        " '%2' port = '%3'")
                     .arg(conn->userName()).arg(conn->hostName())
                     .arg(conn->portNumber()), job.moleQueueId());
    conn->deleteLater();
    return;
  }
}

void QueueRemote::remoteDirectoryCleaned()
{
  SshConnection *conn = qobject_cast<SshConnection*>(sender());
  if (!conn) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not an SshConnection!"));
    return;
  }
  conn->deleteLater();

  Job job = conn->data().value<Job>();

  if (!job.isValid()) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender does not have an associated job!"));
    return;
  }

  if (conn->exitCode() != 0) {
    Logger::logError(tr("Error clearing remote directory '%1@%2:%3/%4'.\n"
                        "Exit code (%5) %6")
                     .arg(conn->userName()).arg(conn->hostName())
                     .arg(m_workingDirectoryBase).arg(job.moleQueueId())
                     .arg(conn->exitCode()).arg(conn->output()),
                     job.moleQueueId());
    job.setJobState(MoleQueue::ErrorState);
    return;
  }
}

void QueueRemote::jobAboutToBeRemoved(const Job &job)
{
  m_pendingSubmission.removeOne(job.moleQueueId());
  Queue::jobAboutToBeRemoved(job);
}

void QueueRemote::beginKillJob(Job job)
{
  const QString command = QString("%1 %2")
      .arg(m_killCommand)
      .arg(job.queueId());

  SshConnection *conn = newSshConnection();
  conn->setData(QVariant::fromValue(job));
  connect(conn, SIGNAL(requestComplete()),
          this, SLOT(endKillJob()));

  if (!conn->execute(command)) {
    Logger::logError(tr("Could not initialize ssh resources: user= '%1'\nhost ="
                        " '%2' port = '%3'")
                     .arg(conn->userName()).arg(conn->hostName())
                     .arg(conn->portNumber()), job.moleQueueId());
    job.setJobState(MoleQueue::ErrorState);
    conn->deleteLater();
    return;
  }
}

void QueueRemote::endKillJob()
{
  SshConnection *conn = qobject_cast<SshConnection*>(sender());
  if (!conn) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not an SshConnection!"));
    return;
  }
  conn->deleteLater();

  Job job = conn->data().value<Job>();
  if (!job.isValid()) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender does not have an associated job!"));
    return;
  }

  if (conn->exitCode() != 0) {
    Logger::logWarning(tr("Error cancelling job (mqid=%1, queueid=%2) on "
                          "%3@%4:%5 (queue=%6)\n(%7) %8")
                       .arg(job.moleQueueId()).arg(job.queueId())
                       .arg(conn->userName()).arg(conn->hostName())
                       .arg(conn->portNumber()).arg(m_name)
                       .arg(conn->exitCode()).arg(conn->output()));
    return;
  }

  job.setJobState(MoleQueue::Killed);
}

SshConnection * QueueRemote::newSshConnection()
{
  SshCommand *command = new SshCommand (this);
  command->setHostName(m_hostName);
  command->setUserName(m_userName);
  command->setPortNumber(m_sshPort);

  return command;
}

void QueueRemote::removeStaleJobs()
{
  if (m_server) {
    if (JobManager *jobManager = m_server->jobManager()) {
      QList<IdType> staleQueueIds;
      for (QMap<IdType, IdType>::const_iterator it = m_jobs.constBegin(),
           it_end = m_jobs.constEnd(); it != it_end; ++it) {
        if (jobManager->lookupJobByMoleQueueId(it.value()).isValid())
          continue;
        staleQueueIds << it.key();
        Logger::logError(tr("Job with MoleQueue id %1 is missing, but the Queue"
                            " '%2' is still holding a reference to it. Please "
                            "report this bug and check if the job needs to be "
                            "resubmitted.").arg(it.value()).arg(name()),
                         it.value());
      }
      foreach (IdType queueId, staleQueueIds)
        m_jobs.remove(queueId);
    }
  }
}

QString QueueRemote::generateQueueRequestCommand()
{
  QList<IdType> queueIds = m_jobs.keys();
  QString queueIdString;
  foreach (IdType id, queueIds) {
    queueIdString += QString::number(id) + " ";
  }

  return QString ("%1 %2").arg(m_requestQueueCommand).arg(queueIdString);
}

void QueueRemote::timerEvent(QTimerEvent *theEvent)
{
  if (theEvent->timerId() == m_checkQueueTimerId) {
    theEvent->accept();
    removeStaleJobs();
    if (!m_jobs.isEmpty())
      requestQueueUpdate();
    return;
  }
  else if (theEvent->timerId() == m_checkForPendingJobsTimerId) {
    theEvent->accept();
    submitPendingJobs();
    return;
  }

  QObject::timerEvent(theEvent);
}

} // End namespace
