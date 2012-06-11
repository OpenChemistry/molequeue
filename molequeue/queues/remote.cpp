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
    m_checkForPendingJobsTimerId(-1)
{
  // Check remote queue every 60 seconds
  m_checkQueueTimerId = this->startTimer(60000);

  // Check for jobs to submit every 5 seconds
  m_checkForPendingJobsTimerId = this->startTimer(5000);
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
  settings.setValue("hostName", m_hostName);
  settings.setValue("userName", m_userName);
  settings.setValue("sshPort",  m_sshPort);
}

QWidget* QueueRemote::settingsWidget()
{
  RemoteQueueWidget *widget = new RemoteQueueWidget (this);
  return widget;
}

bool QueueRemote::submitJob(const Job *job)
{
  m_pendingSubmission.append(job->moleQueueId());

  emit jobStateUpdate(job->moleQueueId(), MoleQueue::Accepted);

  return true;
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
    qWarning() << Q_FUNC_INFO << "Cannot locate jobmanager.";
    return;
  }

  foreach (const IdType moleQueueId, m_pendingSubmission) {
    const Job *job = jobManager->lookupMoleQueueId(moleQueueId);
    // Kick off the submission process...
    this->beginJobSubmission(job);
  }

  m_pendingSubmission.clear();
}

void QueueRemote::beginJobSubmission(const Job *job)
{
  this->writeInputFiles(job);

  this->createRemoteDirectory(job);
}

void QueueRemote::createRemoteDirectory(const Job *job)
{
  // Note that this is just the working directory base -- the job folder is
  // created by scp.
  QString remoteDir = QString("%1").arg(m_workingDirectoryBase);

  SshConnection *conn = this->newSshConnection();
  conn->setData(QVariant::fromValue(job));
  connect(conn, SIGNAL(requestComplete()), this, SLOT(remoteDirectoryCreated()));

  conn->execute(QString("mkdir -p %1").arg(remoteDir));
}

void QueueRemote::remoteDirectoryCreated()
{
  SshConnection *conn = qobject_cast<SshConnection*>(this->sender());
  if (!conn) {
    qWarning() << Q_FUNC_INFO << "sender is not an SshConnection";
    return;
  }

  const Job *job = conn->data().value<const Job*>();

  if (!job) {
    qWarning() << Q_FUNC_INFO << "sender does not have an associated job!";
    return;
  }

  /// @todo Check for errors
  conn->deleteLater();

  this->copyInputFilesToHost(job);
}

void QueueRemote::copyInputFilesToHost(const Job *job)
{
  QString localDir = job->localWorkingDirectory();
  QString remoteDir =
      QString("%1/%2").arg(m_workingDirectoryBase).arg(job->moleQueueId());

  SshConnection *conn = this->newSshConnection();
  conn->setData(QVariant::fromValue(job));
  connect(conn, SIGNAL(requestComplete()), this, SLOT(inputFilesCopied()));

  conn->copyDirTo(localDir, remoteDir);
}

void QueueRemote::inputFilesCopied()
{
  SshConnection *conn = qobject_cast<SshConnection*>(this->sender());
  if (!conn) {
    qWarning() << Q_FUNC_INFO << "sender is not an SshConnection";
    return;
  }

  const Job *job = conn->data().value<const Job*>();

  /// @todo Check for errors
  conn->deleteLater();

  if (!job) {
    qWarning() << Q_FUNC_INFO << "sender does not have an associated job!";
    return;
  }

  this->submitJobToRemoteQueue(job);
}

void QueueRemote::submitJobToRemoteQueue(const Job *job)
{
  const QString command = QString("%1 %2/%3/%4")
      .arg(m_submissionCommand)
      .arg(m_workingDirectoryBase)
      .arg(job->moleQueueId())
      .arg(m_launchScriptName);

  SshConnection *conn = this->newSshConnection();
  conn->setData(QVariant::fromValue(job));
  connect(conn, SIGNAL(requestComplete()),
          this, SLOT(jobSubmittedToRemoteQueue()));

  conn->execute(command);
}

void QueueRemote::jobSubmittedToRemoteQueue()
{
  SshConnection *conn = qobject_cast<SshConnection*>(this->sender());
  if (!conn) {
    qWarning() << Q_FUNC_INFO << "sender is not an SshConnection";
    return;
  }

  /// @todo Check for errors
  IdType queueId;
  this->parseQueueId(conn->output(), &queueId);
  const Job *job = conn->data().value<const Job*>();

  conn->deleteLater();

  if (!job) {
    qWarning() << Q_FUNC_INFO << "sender does not have an associated job!";
    return;
  }

  emit jobStateUpdate(job->moleQueueId(), MoleQueue::Submitted);
  emit queueIdUpdate(job->moleQueueId(), queueId);
  m_jobs.insert(queueId, job->moleQueueId());
}

void QueueRemote::requestQueueUpdate()
{
  if (m_isCheckingQueue)
    return;

  m_isCheckingQueue = true;

  // Checking by job ids doesn't work with e.g. SGE. Use -u instead.
//  QList<IdType> queueIds = m_jobs.keys();
//  QString queueIdString;
//  foreach (IdType id, queueIds) {
//    queueIdString += QString::number(id) + " ";
//  }

//  const QString command = QString ("%1 %2")
//      .arg(m_requestQueueCommand)
//      .arg(queueIdString);
    const QString command = QString ("%1 -u %2")
        .arg(m_requestQueueCommand)
        .arg(m_userName);

  SshConnection *conn = this->newSshConnection();
  connect(conn, SIGNAL(requestComplete()),
          this, SLOT(handleQueueUpdate()));

  conn->execute(command);
}

void QueueRemote::handleQueueUpdate()
{
  SshConnection *conn = qobject_cast<SshConnection*>(this->sender());
  if (!conn) {
    qWarning() << Q_FUNC_INFO << "sender is not an SshConnection";
    return;
  }

  QStringList output = conn->output().split("\n", QString::SkipEmptyParts);

  /// @todo handle error
  conn->deleteLater();

  // Get list of submitted queue ids so that we detect when jobs have left
  // the queue.
  QList<IdType> queueIds = m_jobs.keys();

  MoleQueue::JobState state;
  foreach (QString line, output) {
    IdType queueId;
    if (this->parseQueueLine(line, &queueId, &state)) {
      IdType moleQueueId = m_jobs.value(queueId, 0);
      if (moleQueueId != 0) {
        queueIds.removeOne(queueId);
        emit jobStateUpdate(moleQueueId, state);
      }
    }
  }

  // Now copy back any jobs that have left the queue
  foreach (IdType queueId, queueIds) {
    this->copyFinishedJobOutputFromHost(queueId);
  }

  m_isCheckingQueue = false;
}

void QueueRemote::copyFinishedJobOutputFromHost(IdType queueId)
{
  IdType moleQueueId = m_jobs.value(queueId, 0);
  if (moleQueueId == 0)
    return;

  m_jobs.remove(queueId);

  // Lookup job
  if (!m_server)
    return;
  const Job *job = m_server->jobManager()->lookupMoleQueueId(moleQueueId);
  if (!job)
    return;

  QString localDir = job->localWorkingDirectory();
  QString remoteDir =
      QString("%1/%2").arg(m_workingDirectoryBase).arg(job->moleQueueId());

  SshConnection *conn = this->newSshConnection();
  conn->setData(QVariant::fromValue(job));
  connect(conn, SIGNAL(requestComplete()),
          this, SLOT(finishedJobOutputCopied()));

  conn->copyDirFrom(remoteDir, localDir);
}

void QueueRemote::finishedJobOutputCopied()
{
  SshConnection *conn = qobject_cast<SshConnection*>(this->sender());
  if (!conn) {
    qWarning() << Q_FUNC_INFO << "sender is not an SshConnection";
    return;
  }

  const Job *job = conn->data().value<const Job*>();

  conn->deleteLater();

  if (!job) {
    qWarning() << Q_FUNC_INFO << "sender does not have an associated job!";
    return;
  }

  /// @todo clean remote directory, etc

  emit jobStateUpdate(job->moleQueueId(), MoleQueue::Finished);
}

SshConnection * QueueRemote::newSshConnection()
{
  SshCommand *command = new SshCommand (this);
  command->setHostName(m_hostName);
  command->setUserName(m_userName);
  command->setPortNumber(m_sshPort);

  return command;
}

void QueueRemote::timerEvent(QTimerEvent *theEvent)
{
  if (theEvent->timerId() == m_checkQueueTimerId) {
    theEvent->accept();
    if (m_jobs.size())
      this->requestQueueUpdate();
    return;
  }
  else if (theEvent->timerId() == m_checkForPendingJobsTimerId) {
    theEvent->accept();
    this->submitPendingJobs();
    return;
  }

  QObject::timerEvent(theEvent);
}

} // End namespace
