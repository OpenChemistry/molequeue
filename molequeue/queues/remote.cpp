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
    m_isCheckingQueue(false)
{
  // Check remote queue every 60 seconds
  m_checkQueueTimerId = this->startTimer(60000);
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
  settings.setValue("hostName", m_userName);
  settings.setValue("userName", m_hostName);
  settings.setValue("sshPort",  m_sshPort);
}

QWidget* QueueRemote::settingsWidget()
{
  RemoteQueueWidget *widget = new RemoteQueueWidget (this);
  return widget;
}

bool QueueRemote::submitJob(const Job *job)
{
  /// @todo This needs to be rewritten
  Q_UNUSED(job);
  return false;
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

  if (!job) {
    qWarning() << Q_FUNC_INFO << "sender does not have an associated job!";
    return;
  }

  /// @todo Check for errors
  conn->deleteLater();

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

  const Job *job = conn->data().value<const Job*>();

  if (!job) {
    qWarning() << Q_FUNC_INFO << "sender does not have an associated job!";
    return;
  }

  /// @todo Check for errors
  IdType queueId;
  this->parseQueueId(conn->output(), &queueId);
  conn->deleteLater();

  emit jobStateUpdate(job->moleQueueId(), MoleQueue::Submitted);
  emit queueIdUpdate(job->moleQueueId(), queueId);
  m_jobs.insert(queueId, job->moleQueueId());
}

void QueueRemote::requestQueueUpdate()
{
  if (m_isCheckingQueue)
    return;

  m_isCheckingQueue = true;

  QList<IdType> queueIds = m_jobs.keys();
  QString queueIdString;
  foreach (IdType id, queueIds) {
    queueIdString += QString::number(id) + " ";
  }

  const QString command = QString ("%1 %2")
      .arg(m_requestQueueCommand)
      .arg(queueIdString);

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

  /// @todo handle error

  QStringList output = conn->output().split("\n", QString::SkipEmptyParts);

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

  QObject::timerEvent(theEvent);
}

} // End namespace
