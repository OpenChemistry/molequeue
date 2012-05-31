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

#ifndef QUEUELOCALWORKER_H
#define QUEUELOCALWORKER_H

#include <QtCore/QObject>

#include "../molequeueglobal.h"

#include <QtCore/QMap>
#include <QtCore/QProcess> // For QProcess::ExitStatus

class QThread;

namespace MoleQueue
{
class Job;
class QueueLocal;

class QueueLocalWorker : public QObject
{
  Q_OBJECT
public:
  explicit QueueLocalWorker(QueueLocal *queue, QThread *workerThread);

  /**
   * Submit a new job.
   * @param job The Job.
   * @return True if the job is accepted, false otherwise.
   */
  bool acceptSubmission(const Job *job);

public slots:

  /**
   * Called when the worker thread starts.
   */
  void makeConnections();

signals:

  /**
   * Emitted once the worker is installed to the worker thread and ready to make
   * connections.
   */
  void readyToConnect();

  /**
   * Emitted when a new job enters this queue.
   * @param job The Job.
   * @note This signal is used to ensure threads behave appropriately. It is for
   * internal use only.
   */
  void prepareJobForSubmission(const MoleQueue::Job *job);

  /**
   * Emitted when a job changes state.
   * @param moleQueueId MoleQueue id of job.
   * @param newState New state of job.
   */
  void jobStateChanged(MoleQueue::IdType moleQueueId,
                       MoleQueue::JobState newState);

protected slots:

  /**
   * Used to move this class's event handling to the worker thread once it
   * starts.
   */
  void moveToWorkerThread();

  /**
   * Handler for prepareJobForSubmission
   * @param job The Job.
   */
  void handlePrepareJobForSubmission(const MoleQueue::Job *job);

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

  bool writeInputFiles(const Job *job);
  bool addJobToQueue(const Job *job);

  /// Connect @a proc to handlers prior to submitting job
  void connectProcess(QProcess *proc);

  bool checkJobLimit();

  bool startJob(IdType moleQueueId);

  void timerEvent(QTimerEvent *theEvent);

  /// Internal timer id.
  int m_checkJobLimitTimerId;

  /// Cached pointer to associated QueueLocal.
  QueueLocal *m_queue;

  /// Cached pointer to worker thread
  QThread *m_thread;

  /// FIFO queue of MoleQueue ids.
  QList<IdType> m_pendingJobQueue;

  /// List of running processes. MoleQueue Id to QProcess*
  QMap<IdType, QProcess*> m_runningJobs;
};

} // end namespace MoleQueue

#endif // QUEUELOCALWORKER_H
