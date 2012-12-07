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

#ifndef QUEUEREMOTE_H
#define QUEUEREMOTE_H

#include "../queue.h"

class QTimer;

namespace MoleQueue
{
class QueueManager;

/// @brief abstract Queue subclass for interacting with a generic Remote queue.
class QueueRemote : public Queue
{
  Q_OBJECT
public:
  explicit QueueRemote(const QString &queueName = "AbstractRemote",
                       QueueManager *parentManager = 0);
  ~QueueRemote();

  bool writeJsonSettings(QJsonObject &json, bool exportOnly,
                         bool includePrograms) const;
  bool readJsonSettings(const QJsonObject &json, bool importOnly,
                        bool includePrograms);

  virtual AbstractQueueSettingsWidget* settingsWidget() = 0;

  void setWorkingDirectoryBase(const QString &base)
  {
    m_workingDirectoryBase = base;
  }

  QString workingDirectoryBase() const
  {
    return m_workingDirectoryBase;
  }

  /** Time between remote queue updates in minutes. */
  void setQueueUpdateInterval(int i);

  /** Time between remote queue updates in minutes. */
  int queueUpdateInterval() const { return m_queueUpdateInterval; }

  /**
   * @brief setDefaultMaxWallTime Set the default walltime limit (in minutes)
   * for jobs on this queue. This value will be used if the job's
   * Job::maxWallTime() method returns a value <= 0. Default is one day.
   * @param time walltime limit in minutes
   * @see Job::maxWallTime()
   */
  void setDefaultMaxWallTime(int time) { m_defaultMaxWallTime = time; }

  /**
   * @brief defaultMaxWallTime Get the default walltime limit (in minutes)
   * for jobs on this queue. This value will be used if the job's
   * Job::maxWallTime() method returns a value <= 0. Default is one day.
   * @return walltime limit in minutes
   * @see Job::maxWallTime()
   */
  int defaultMaxWallTime() const { return m_defaultMaxWallTime; }

  /// Reimplemented from Queue::replaceKeywords
  void replaceKeywords(QString &launchScript, const Job &job,
                       bool addNewline = true);

public slots:

  bool submitJob(MoleQueue::Job job);
  void killJob(MoleQueue::Job job);

  virtual void requestQueueUpdate() = 0;

protected slots:
  virtual void submitPendingJobs();

  /// Main entry point into the job submission pipeline
  virtual void beginJobSubmission(MoleQueue::Job job);

  virtual void createRemoteDirectory(MoleQueue::Job job) = 0;
  virtual void remoteDirectoryCreated() = 0;

  virtual void copyInputFilesToHost(MoleQueue::Job job) = 0;
  virtual void inputFilesCopied() = 0;
  virtual void submitJobToRemoteQueue(MoleQueue::Job job) = 0;
  virtual void jobSubmittedToRemoteQueue() = 0;

  virtual void handleQueueUpdate() = 0;

  virtual void beginFinalizeJob(MoleQueue::IdType queueId) = 0;
  virtual void finalizeJobCopyFromServer(MoleQueue::Job job) = 0;
  virtual void finalizeJobOutputCopiedFromServer() = 0;
  virtual void finalizeJobCopyToCustomDestination(MoleQueue::Job job) = 0;
  virtual void finalizeJobCleanup(MoleQueue::Job job);

  virtual void cleanRemoteDirectory(MoleQueue::Job job) = 0;
  virtual void remoteDirectoryCleaned() = 0;

  /// Reimplemented from Queue
  void jobAboutToBeRemoved(const MoleQueue::Job &job);

  virtual void beginKillJob(MoleQueue::Job job) = 0;
  virtual void endKillJob() = 0;

protected:

  /**
   * Check for any jobs that are not present in the JobManager but
   * are still in this object's internal data structures. This may be the result
   * of an improper shut down when state is serialized inconsistently. If any
   * such jobs are found, they are removed from the internal structures and an
   * Error is emitted.
   */
  virtual void removeStaleJobs();

  /// Reimplemented to monitor queue events.
  virtual void timerEvent(QTimerEvent *theEvent);

  int m_checkQueueTimerId;

  /// MoleQueue ids of jobs that have been accepted but not submitted.
  QList<IdType> m_pendingSubmission;
  int m_checkForPendingJobsTimerId;

  /// Time between remote queue updates in minutes.
  int m_queueUpdateInterval;

  /// Default maximum walltime limit for jobs on this queue in minutes.
  int m_defaultMaxWallTime;

  QString m_workingDirectoryBase;
};

} // End namespace

#endif // QUEUEREMOTE_H
