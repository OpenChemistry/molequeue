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
class SshConnection;

/// @brief Queue subclass for interacting with a generic Remote queue.
class QueueRemote : public Queue
{
  Q_OBJECT
public:
  explicit QueueRemote(const QString &queueName = "AbstractRemote",
                       QueueManager *parentManager = 0);
  ~QueueRemote();

  /**
   * Read settings for the queue, done early on at startup.
   */
  virtual void readSettings(QSettings &settings);

  /**
   * Write settings for the queue, done just before closing the server.
   */
  virtual void writeSettings(QSettings &settings) const;

  /**
   * Populate the passed QSettings object with this queue's configuration.
   * Sensitive data (such as usernames, etc) and mutatable state data (like
   * current jobs) are not written, see writeSettings() if these are needed.
   * @param includePrograms Export this queue's programs as well. Default: true
   */
  void exportConfiguration(QSettings &exporter,
                           bool includePrograms = true) const;

  /**
   * Set this Queue's configuration from the passed QSettings object.
   * Sensitive data (such as usernames, etc) and mutatable state data (like
   * current jobs) are not read, see writeSettings() if these are needed.
   * @param includePrograms Import any programs contained in the importer.
   * Default: true
   */
  void importConfiguration(QSettings &importer,
                           bool includePrograms = true);

  virtual AbstractQueueSettingsWidget* settingsWidget();

  void setSshExecutable(const QString &exe)
  {
    m_sshExecutable = exe;
  }

  QString sshExecutable() const
  {
    return m_sshExecutable;
  }

  void setScpExecutable(const QString &exe)
  {
    m_scpExecutable = exe;
  }

  QString scpExectuable() const
  {
    return m_scpExecutable;
  }

  void setHostName(const QString &host)
  {
    m_hostName = host;
  }

  QString hostName() const
  {
    return m_hostName;
  }

  void setUserName(const QString &user)
  {
    m_userName = user;
  }

  QString userName() const
  {
    return m_userName;
  }

  void setSshPort(int port)
  {
    m_sshPort = port;
  }

  int sshPort() const
  {
    return m_sshPort;
  }

  void setWorkingDirectoryBase(const QString &base)
  {
    m_workingDirectoryBase = base;
  }

  QString workingDirectoryBase() const
  {
    return m_workingDirectoryBase;
  }

  void setSubmissionCommand(const QString &command)
  {
    m_submissionCommand = command;
  }

  QString submissionCommand() const
  {
    return m_submissionCommand;
  }

  void setKillCommand(const QString &command)
  {
    m_killCommand = command;
  }

  QString killCommand() const
  {
    return m_killCommand;
  }

  void setRequestQueueCommand(const QString &command)
  {
    m_requestQueueCommand = command;
  }

  QString requestQueueCommand() const
  {
    return m_requestQueueCommand;
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

  /// Reimplemented from Queue::replaceLaunchScriptKeywords
  void replaceLaunchScriptKeywords(QString &launchScript, const Job &job);

public slots:

  bool submitJob(MoleQueue::Job job);
  void killJob(MoleQueue::Job job);

protected slots:
  virtual void submitPendingJobs();

  /// Main entry point into the job submission pipeline
  virtual void beginJobSubmission(MoleQueue::Job job);

  virtual void createRemoteDirectory(MoleQueue::Job job);
  virtual void remoteDirectoryCreated();

  virtual void copyInputFilesToHost(MoleQueue::Job job);
  virtual void inputFilesCopied();
  virtual void submitJobToRemoteQueue(MoleQueue::Job job);
  virtual void jobSubmittedToRemoteQueue();

  virtual void requestQueueUpdate();
  virtual void handleQueueUpdate();

  virtual void beginFinalizeJob(MoleQueue::IdType queueId);
  virtual void finalizeJobCopyFromServer(MoleQueue::Job job);
  virtual void finalizeJobOutputCopiedFromServer();
  virtual void finalizeJobCopyToCustomDestination(MoleQueue::Job job);
  virtual void finalizeJobCleanup(MoleQueue::Job job);

  virtual void cleanRemoteDirectory(MoleQueue::Job job);
  virtual void remoteDirectoryCleaned();

  /// Reimplemented from Queue
  void jobAboutToBeRemoved(const MoleQueue::Job &job);

  virtual void beginKillJob(MoleQueue::Job job);
  virtual void endKillJob();

protected:
  virtual SshConnection *newSshConnection();

  /**
   * Check for any jobs that are not present in the JobManager but
   * are still in this object's internal data structures. This may be the result
   * of an improper shut down when state is serialized inconsistently. If any
   * such jobs are found, they are removed from the internal structures and an
   * Error is emitted.
   */
  virtual void removeStaleJobs();

  /**
   * Extract the job id from the submission output. Reimplement this in derived
   * classes.
   * @param submissionOutput Output from m_submissionCommand
   * @param queueId The queuing system's job id.
   * @return True if parsing successful, false otherwise.
   */
  virtual bool parseQueueId(const QString &submissionOutput, IdType *queueId) = 0;

  /**
   * Prepare the command to check the remote queue. The default implementation
   * is m_requestQueueCommand followed by the owned job ids separated by
   * spaces.
   */
  virtual QString generateQueueRequestCommand();

  /**
   * Extract the queueId and JobState from a single line of the the queue list
   * output. Reimplement this in derived classes.
   * @param queueListOutput Single line of output from m_requestQueueCommand
   * @param queueId The queuing system's job id.
   * @param state The state of the job with id queueId
   * @return True if parsing successful, false otherwise.
   */
  virtual bool parseQueueLine(const QString &queueListOutput, IdType *queueId,
                              MoleQueue::JobState *state) = 0;

  /// Reimplemented to monitor queue events.
  virtual void timerEvent(QTimerEvent *theEvent);

  QString m_sshExecutable;
  QString m_scpExecutable;
  QString m_hostName;
  QString m_userName;
  int m_sshPort;
  int m_checkQueueTimerId;
  bool m_isCheckingQueue;
  /// MoleQueue ids of jobs that have been accepted but not submitted.
  QList<IdType> m_pendingSubmission;
  int m_checkForPendingJobsTimerId;

  QString m_submissionCommand;
  QString m_killCommand;
  QString m_requestQueueCommand;

  /// Time between remote queue updates in minutes.
  int m_queueUpdateInterval;

  /// Default maximum walltime limit for jobs on this queue in minutes.
  int m_defaultMaxWallTime;

  /// List of allowed exit codes for m_requestQueueCommand. This is required for
  /// e.g. PBS/Torque, which return 153 if you request the status of a job that
  /// has completed.
  QList<int> m_allowedQueueRequestExitCodes;

  QString m_workingDirectoryBase;
};

} // End namespace

#endif // QUEUEREMOTE_H
