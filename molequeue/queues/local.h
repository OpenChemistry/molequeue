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

#ifndef QUEUELOCAL_H
#define QUEUELOCAL_H

#include "../queue.h"

#include <QtCore/QProcess>

class QThread;

namespace MoleQueue
{
class Job;
class QueueManager;

/// @brief Queue for running jobs locally.
class QueueLocal : public Queue
{
  Q_OBJECT
public:
  explicit QueueLocal(QueueManager *parentManager = 0);
  ~QueueLocal();

  QString typeName() const { return "Local"; }

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

  /**
   * Returns a widget that can be used to configure the settings for the
   * queue.
   */
  AbstractQueueSettingsWidget* settingsWidget();

  /// The number of cores available.
  int maxNumberOfCores() const;

  /// The number of cores available.
  void setMaxNumberOfCores(int cores) { m_cores = cores; }

public slots:
  bool submitJob(MoleQueue::Job job);
  void killJob(MoleQueue::Job job);

protected slots:
  /**
   * Write the input files for the job and add to the queue
   * @param job The Job.
   * @return True on success, false otherwise.
   */
  bool prepareJobForSubmission(const MoleQueue::Job &job);

  /**
   * Called when a process starts.
   */
  void processStarted();

  /**
   * Called when a process exits.
   * @param exitCode Exit code of process
   * @param exitStatus Exit status of process
   */
  void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

protected:
  /// Insert the job into the queue.
  bool addJobToQueue(const Job &job);

  /// Connect @a proc to handlers prior to submitting job
  void connectProcess(QProcess *proc);

  /// Test if any more jobs can be submitted.
  bool checkJobLimit();

  /// Submit the job with MoleQueue id @a moleQueueId.
  bool startJob(IdType moleQueueId);

  /// Reimplemented to monitor queue events.
  void timerEvent(QTimerEvent *theEvent);

  /// Internal timer id.
  int m_checkJobLimitTimerId;

  /// FIFO queue of MoleQueue ids.
  QList<IdType> m_pendingJobQueue;

  /// List of running processes. MoleQueue Id to QProcess*
  QMap<IdType, QProcess*> m_runningJobs;

  /// The number of cores available.
  int m_cores;
};

} // End namespace

#endif // QUEUELOCAL_H
