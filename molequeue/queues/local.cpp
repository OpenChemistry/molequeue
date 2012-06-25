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

#include "local.h"

#include "../job.h"
#include "../jobmanager.h"
#include "../program.h"
#include "../queue.h"
#include "../queuemanager.h"
#include "../server.h"

#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QTimerEvent>
#include <QtCore/QThread> // For ideal thread count

#include <QtGui/QWidget>
#include <QtGui/QFormLayout>
#include <QtGui/QSpinBox>

#include <QtCore/QDebug>

#ifdef WIN32
#include <Windows.h> // For _PROCESS_INFORMATION (PID parsing)
#endif

namespace MoleQueue {

QueueLocal::QueueLocal(QueueManager *parentManager) :
  Queue("Local", parentManager),
  m_checkJobLimitTimerId(-1),
  m_cores(-1)
{
#ifdef WIN32
  m_launchTemplate = "@echo off\n\n$$programExecution$$\n";
  m_launchScriptName = "MoleQueueLauncher.bat";
#else // WIN32
  m_launchTemplate = "#!/bin/bash\n\n$$programExecution$$\n";
  m_launchScriptName = "MoleQueueLauncher.sh";
#endif // WIN32

  // Check if new jobs need starting every 10 seconds
  m_checkJobLimitTimerId = this->startTimer(10000);
}

QueueLocal::~QueueLocal()
{
}

void QueueLocal::readSettings(QSettings &settings)
{
  Queue::readSettings(settings);
  m_cores = settings.value("cores", -1).toInt();

  // use all cores if no value set
  if(m_cores == -1){
    m_cores = QThread::idealThreadCount();
  }

  int numPendingJobs = settings.beginReadArray("PendingJobs");
  m_pendingJobQueue.reserve(numPendingJobs);
  for (int i = 0; i < numPendingJobs; ++i) {
    settings.setArrayIndex(i);
    IdType mqId = settings.value("moleQueueId", 0).value<IdType>();
    if (mqId == 0)
      continue;
    m_pendingJobQueue.append(mqId);
  }
  settings.endArray(); // "PendingJobs"
}

void QueueLocal::writeSettings(QSettings &settings) const
{
  Queue::writeSettings(settings);
  settings.setValue("cores", m_cores);

  QList<IdType> jobsToResume;
  jobsToResume.append(m_runningJobs.keys());
  jobsToResume.append(m_pendingJobQueue);
  settings.beginWriteArray("PendingJobs", jobsToResume.size());
  for (int i = 0; i < jobsToResume.size(); ++i) {
    settings.setArrayIndex(i);
    settings.setValue("moleQueueId", jobsToResume.at(i));
  }
  settings.endArray(); // "PendingJobs"
}

QWidget* QueueLocal::settingsWidget()
{
  QWidget *widget = new QWidget;

  QFormLayout *layout = new QFormLayout;
  QSpinBox *coresSpinBox = new QSpinBox(widget);
  coresSpinBox->setValue(m_cores);
  coresSpinBox->setMinimum(0);
  layout->addRow("Number of Cores: ", coresSpinBox);
  widget->setLayout(layout);

  return widget;
}

bool QueueLocal::submitJob(const Job *job)
{
  emit jobStateUpdate(job->moleQueueId(), MoleQueue::Accepted);
  this->prepareJobForSubmission(job);
  return true;
}

bool QueueLocal::prepareJobForSubmission(const Job *job)
{
  if (!this->writeInputFiles(job))
    return false;
  if (!this->addJobToQueue(job))
    return false;

  return true;
}

void QueueLocal::processStarted()
{
  QProcess *process = qobject_cast<QProcess*>(this->sender());
  if (!process)
    return;

  IdType moleQueueId = m_runningJobs.key(process, 0);
  if (moleQueueId == 0)
    return;

  IdType queueId;
#ifdef WIN32
  queueId = static_cast<IdType>(process->pid()->dwProcessId);
#else // WIN32
  queueId = static_cast<IdType>(process->pid());
#endif // WIN32

  emit queueIdUpdate(moleQueueId, queueId);
  emit jobStateUpdate(moleQueueId, MoleQueue::RunningLocal);
}

void QueueLocal::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  Q_UNUSED(exitCode);
  Q_UNUSED(exitStatus);

  QProcess *process = qobject_cast<QProcess*>(this->sender());
  if (!process)
    return;

  IdType moleQueueId = m_runningJobs.key(process, 0);
  if (moleQueueId == 0)
    return;

  // Remove and delete QProcess from queue
  m_runningJobs.take(moleQueueId)->deleteLater();

  emit jobStateUpdate(moleQueueId, MoleQueue::Finished);
}

int QueueLocal::cores() const
{
  if (m_cores > 0)
    return m_cores;
  else
    return QThread::idealThreadCount() > 8 ? 8 : QThread::idealThreadCount();
}


bool QueueLocal::addJobToQueue(const Job *job)
{
  m_pendingJobQueue.append(job->moleQueueId());

  emit jobStateUpdate(job->moleQueueId(), MoleQueue::LocalQueued);

  return true;
}

void QueueLocal::connectProcess(QProcess *proc)
{
  connect(proc, SIGNAL(started()),
          this, SLOT(processStarted()));
  connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
          this, SLOT(processFinished(int,QProcess::ExitStatus)));
}

bool QueueLocal::checkJobLimit()
{
  if (m_runningJobs.isEmpty() && !m_pendingJobQueue.isEmpty()) {
    if (!this->startJob(m_pendingJobQueue.takeFirst())) {
      return false;
    }
  }

  return true;
}

bool QueueLocal::startJob(IdType moleQueueId)
{
  // Get pointers to job, server, etc
  if (!m_server) {
    qWarning() << Q_FUNC_INFO << "Error: Cannot locate server.";
    return false;
  }
  const Job *job = m_server->jobManager()->lookupMoleQueueId(moleQueueId);
  if (!job) {
    qWarning() << Q_FUNC_INFO << "Error: Unrecognized MoleQueue id:"
               << moleQueueId;
    return false;
  }
  const Queue *queue = m_server->queueManager()->lookupQueue(job->queue());
  if (!queue) {
    qWarning() << Q_FUNC_INFO << "Error: Unknown queue:" << job->queue();
    return false;
  }
  const Program *program = queue->lookupProgram(job->program());
  if (!queue) {
    qWarning() << Q_FUNC_INFO << "Error: Unknown program:" << job->program();
    return false;
  }

  // Create and setup process
  QProcess *proc = new QProcess (this);
  QDir dir (job->localWorkingDirectory());
  proc->setWorkingDirectory(dir.absolutePath());

  QStringList arguments;
  if (program->arguments().isEmpty())
    arguments << program->arguments();

  QString command;

  // Set default command. May be overwritten later.
  if (program->useExecutablePath())
    command = program->executablePath() + "/" + program->executable();
  else
    command = program->executable();

  switch (program->launchSyntax()) {
  case Program::CUSTOM:
    /// @todo batch script on windows
    command = "./MoleQueueLauncher.sh";
    break;
  case Program::PLAIN:
    break;
  case Program::INPUT_ARG:
    arguments << program->inputFilename();
    break;
  case Program::INPUT_ARG_NO_EXT:
    arguments << program->inputFilenameNoExtension();
    break;
  case Program::REDIRECT:
    proc->setStandardInputFile(dir.absoluteFilePath(program->inputFilename()));
    proc->setStandardOutputFile(dir.absoluteFilePath(program->outputFilename()));
    break;
  case Program::INPUT_ARG_OUTPUT_REDIRECT:
    arguments << program->inputFilename();
    proc->setStandardOutputFile(dir.absoluteFilePath(program->outputFilename()));
    break;
  case Program::SYNTAX_COUNT:
  default:
    qWarning() << Q_FUNC_INFO << "Error: Unknown launch syntax:"
               << program->launchSyntax();
    return false;
  }

  this->connectProcess(proc);

  qDebug() << "Starting process:" << command + arguments.join(" ");
  qDebug() << "workingdir:" <<  proc->workingDirectory();
  // This next line *should* work, but doesn't....?
//  proc->start(command, arguments);
  proc->start(command + arguments.join(" "));
  m_runningJobs.insert(job->moleQueueId(), proc);

  return true;
}

void QueueLocal::timerEvent(QTimerEvent *theEvent)
{
  if (theEvent->timerId() == m_checkJobLimitTimerId) {
    if (!this->checkJobLimit()) {
      qWarning() << Q_FUNC_INFO << "Error checking queue...";
    }
    theEvent->accept();
    return;
  }

  QObject::timerEvent(theEvent);
}

} // End namespace
