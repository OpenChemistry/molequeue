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

#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include <QtGui>

namespace MoleQueue {

QueueRemote::QueueRemote(const QString &queueName, QueueManager *parentObject)
  : Queue(queueName, parentObject),
    m_isCheckingQueue(false),
    m_checkForPendingJobsTimerId(-1),
    m_queueUpdateInterval(DEFAULT_REMOTE_QUEUE_UPDATE_INTERVAL),
    m_defaultMaxWallTime(DEFAULT_MAX_WALLTIME)
{
  // Set remote queue check timer.
  m_checkQueueTimerId = startTimer(m_queueUpdateInterval * 60000);

  // Check for jobs to submit every 5 seconds
  m_checkForPendingJobsTimerId = startTimer(5000);
}

QueueRemote::~QueueRemote()
{
}

void QueueRemote::readSettings(QSettings &settings)
{
  Queue::readSettings(settings);

  m_workingDirectoryBase = settings.value("workingDirectoryBase").toString();
  m_queueUpdateInterval =
      settings.value("queueUpdateInterval",
                     DEFAULT_REMOTE_QUEUE_UPDATE_INTERVAL).toInt();
  m_defaultMaxWallTime = settings.value("defaultMaxWallTime",
                                        DEFAULT_MAX_WALLTIME).toInt();
}

void QueueRemote::writeSettings(QSettings &settings) const
{
  Queue::writeSettings(settings);

  settings.setValue("workingDirectoryBase", m_workingDirectoryBase);
  settings.setValue("queueUpdateInterval", m_queueUpdateInterval);
  settings.setValue("defaultMaxWallTime", m_defaultMaxWallTime);
}

void QueueRemote::exportConfiguration(QSettings &exporter,
                                      bool includePrograms) const
{
  Queue::exportConfiguration(exporter, includePrograms);

  exporter.setValue("queueUpdateInterval", m_queueUpdateInterval);
  exporter.setValue("defaultMaxWallTime", m_defaultMaxWallTime);
}

void QueueRemote::importConfiguration(QSettings &importer,
                                      bool includePrograms)
{
  Queue::importConfiguration(importer, includePrograms);

  m_queueUpdateInterval =
      importer.value("queueUpdateInterval",
                     DEFAULT_REMOTE_QUEUE_UPDATE_INTERVAL).toInt();
  m_defaultMaxWallTime = importer.value("defaultMaxWallTime",
                                        DEFAULT_MAX_WALLTIME).toInt();
}

void QueueRemote::setQueueUpdateInterval(int interval)
{
  if (interval == m_queueUpdateInterval)
    return;

  m_queueUpdateInterval = interval;

  killTimer(m_checkQueueTimerId);
  m_checkQueueTimerId = startTimer(m_queueUpdateInterval * 60000);
  requestQueueUpdate();
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
  if (!writeInputFiles(job)) {
    Logger::logError(tr("Error while writing input files."), job.moleQueueId());
    job.setJobState(Error);
    return;
  }
  // Attempt to copy the files via scp first. Only call mkdir on the remote
  // working directory if the scp call fails.
  copyInputFilesToHost(job);
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
    job.setJobState(MoleQueue::Error);
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


void QueueRemote::jobAboutToBeRemoved(const Job &job)
{
  m_pendingSubmission.removeOne(job.moleQueueId());
  Queue::jobAboutToBeRemoved(job);
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
