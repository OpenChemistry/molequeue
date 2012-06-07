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

#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QtCore/QObject>

#include "molequeueglobal.h"

#include <QtCore/QVariantHash>

class QSettings;

class ClientTest;

namespace MoleQueue
{
class Job;

/**
 * @class JobManager jobmanager.h <molequeue/jobmanager.h>
 * @brief Owns and manages Job objects.
 * @author David C. Lonie
 *
 * The JobManager class owns all Job objects. At least two JobManager objects
 * exist during normal operation; the Client class holds a JobManager to track
 * all jobs belonging to that client, and the MainWindow class of the MoleQueue
 * server holds a JobManager to track all jobs that it is managing.
 */
class JobManager : public QObject
{
  Q_OBJECT
public:
  explicit JobManager(QObject *parentObject = 0);
  virtual ~JobManager();

  /// @param settings QSettings object to write state to.
  void readSettings(QSettings &settings);
  /// @param settings QSettings object to read state from.
  void writeSettings(QSettings &settings) const;

  /**
   * @return A new Job object, set to default values.
   */
  Job * newJob();

  /**
   * @param jobState A QVariantHash describing the state of the new Job.
   * @return A new Job object, initialized to the state in @a jobState.
   * @sa Job::hash() Job::setFromHash()
   */
  Job * newJob(const QVariantHash &jobState);

  /**
   * Remove the specified @a job from this manager and delete it.
   */
  void removeJob(const Job* job);

  /**
   * Remove the job with the specified @a moleQueueId from this manager and
   * delete it.
   */
  void removeJob(IdType moleQueueId);

  /**
   * Remove the specified @a jobs from this manager and delete them.
   */
  void removeJobs(const QList<const Job*> &jobsToRemove);

  /**
   * Remove the jobs with the specified @a moleQueueIds from this manager and
   * delete them.
   */
  void removeJobs(const QList<IdType> &moleQueueIds);

  /**
   * @param clientId The Client Id of the requested Job.
   * @return The Job with the requested client id, or NULL if none found.
   * @warning This function is intended to be used from client-side only, as
   * the server may contain multiple clients with redundant ids.
   */
  const Job * lookupClientId(IdType clientId) const;

  /**
   * @param moleQueueId The MoleQueue Id of the requested Job.
   * @return The Job with the requested MoleQueue Id, or NULL if none found.
   */
  const Job * lookupMoleQueueId(IdType moleQueueId) const;

  /**
   * @return A list of all Job objects owned by this JobManager.
   */
  QList<const Job*> jobs() const {return m_jobs;}

  /**
   * Return a list of Job objects that have JobState @a state.
   * @param state JobState of interests
   * @return List of Job objects with JobState @a state
   */
  QList<const Job*> jobsWithJobState(MoleQueue::JobState state);

  /**
   * Return the job at the specified index.
   * @param index Index of Job.
   * @return Requested Job object.
   */
  const Job * jobAt(int index) const
  {
    if (index >= m_jobs.size() || index < 0)
      return NULL;
    return m_jobs[index];
  }

  /**
   * @return Number of Job objects held by this manager.
   */
  int count() const {return m_jobs.size();}

  /// Used for unit tests
  friend class ::ClientTest;

public slots:

  /**
   * Inform the QueueManager that the MoleQueue id or Client id of @a job have
   * changed so that it may update its internal lookup tables.
   * @param job The Job object.
   */
  void jobIdsChanged(const MoleQueue::Job *job);

  /**
   * Called when a job enters a new state. If the new state differs from the
   * previous state, the Job object is updated and the jobStateChanged signal
   * is emitted.
   * @param moleQueueId
   * @param newState
   */
  void updateJobState(MoleQueue::IdType moleQueueId,
                      MoleQueue::JobState newState);

  /**
   * Called when a job is assigned a QueueId by the queue
   * @param moleQueueId
   * @param queueId
   */
  void updateQueueId(MoleQueue::IdType moleQueueId,
                     MoleQueue::IdType queueId);

signals:

  /**
   * Emitted when a job is about to be inserted. Client and MainWindow should
   * directly connect slots to this signal which will set the client or
   * molequeue ids.
   */
  void jobAboutToBeAdded(MoleQueue::Job *);

  /**
   * Emitted when a Job has been added to this JobManager.
   * @param job The new Job object.
   */
  void jobAdded(const MoleQueue::Job *job);

  /**
   * Emitted when a Job changes JobState.
   * @param job Job object
   * @param oldState Previous state of @a job
   * @param newState New state of @a job
   */
  void jobStateChanged(const MoleQueue::Job *job,
                       MoleQueue::JobState oldState,
                       MoleQueue::JobState newState);

  /**
   * Emitted when a Job is assigned a queue id.
   * @param job
   * @param queueId
   */
  void queueIdChanged(const MoleQueue::Job *job,
                      MoleQueue::IdType queueId);

  /**
   * Emitted when the @a job is about to be removed and deleted.
   */
  void jobAboutToBeRemoved(const MoleQueue::Job *job);

  /**
   * Emitted when the @a job with the specified @a moleQueueId has been removed
   * and deleted.
   *
   * @warning Do not dereference @a job, as it no longer points to allocated
   * memory.
   */
  void jobRemoved(MoleQueue::IdType moleQueueId, const MoleQueue::Job *job);

protected:
  /// @param job Job to insert into the internal lookup structures.
  void insertJob(Job *job);

  /// List of all jobs.
  QList<const Job*> m_jobs;

  /// Lookup table for client ids
  QMap<IdType, const Job*> m_clientMap;

  /// Lookup table for MoleQueue ids
  QMap<IdType, const Job*> m_moleQueueMap;
};

}

#endif // JOBMANAGER_H
