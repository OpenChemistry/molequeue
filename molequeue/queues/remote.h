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

/**
 * Remote queue.
 */

class QueueRemote : public Queue
{
  Q_OBJECT
public:
  explicit QueueRemote(const QString &queueName = "AbstractRemote",
                       QueueManager *parentManager = 0);
  ~QueueRemote();

  virtual QString typeName() const { return "Remote"; }

  /**
   * Read settings for the queue, done early on at startup.
   */
  virtual void readSettings(QSettings &settings);

  /**
   * Write settings for the queue, done just before closing the server.
   */
  virtual void writeSettings(QSettings &settings) const;

  virtual QWidget *settingsWidget();

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

  void setRequestQueueCommand(const QString &command)
  {
    m_requestQueueCommand = command;
  }

  QString requestQueueCommand() const
  {
    return m_requestQueueCommand;
  }

public slots:

  virtual bool submitJob(const MoleQueue::Job *job);

protected slots:
  virtual void submitPendingJobs();

  /// Main entry point into the job submission pipeline
  virtual void beginJobSubmission(const MoleQueue::Job *job);

  virtual void createRemoteDirectory(const MoleQueue::Job *job);
  virtual void remoteDirectoryCreated();

  virtual void copyInputFilesToHost(const MoleQueue::Job *job);
  virtual void inputFilesCopied();
  virtual void submitJobToRemoteQueue(const MoleQueue::Job *job);
  virtual void jobSubmittedToRemoteQueue();

  virtual void requestQueueUpdate();
  virtual void handleQueueUpdate();
  virtual void copyFinishedJobOutputFromHost(MoleQueue::IdType queueId);
  virtual void finishedJobOutputCopied();

  virtual void cleanRemoteDirectory(const MoleQueue::Job *job);
  virtual void remoteDirectoryCleaned();

protected:
  SshConnection *newSshConnection();

  /**
   * Extract the job id from the submission output. Reimplement this in derived
   * classes.
   * @param submissionOutput Output from m_submissionCommand
   * @param queueId The queuing system's job id.
   * @return True if parsing successful, false otherwise.
   */
  virtual bool parseQueueId(const QString &submissionOutput, IdType *queueId) = 0;

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

  QString m_hostName;
  QString m_userName;
  int m_sshPort;
  int m_checkQueueTimerId;
  bool m_isCheckingQueue;
  /// MoleQueue ids of jobs that have been accepted but not submitted.
  QList<IdType> m_pendingSubmission;
  int m_checkForPendingJobsTimerId;

  QString m_submissionCommand;
  QString m_requestQueueCommand;

  QString m_workingDirectoryBase;
};

} // End namespace

#endif // QUEUEREMOTE_H
