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

#include "jobmanager.h"

#include "job.h"

#include <QtCore/QSettings>

namespace MoleQueue
{

JobManager::JobManager(QObject *parentObject) :
  QObject(parentObject)
{
  qRegisterMetaType<Job*>("MoleQueue::Job*");
  qRegisterMetaType<const Job*>("const MoleQueue::Job*");
}

JobManager::~JobManager()
{
  m_clientMap.clear();
  m_moleQueueMap.clear();
  qDeleteAll(m_jobs);
  m_jobs.clear();
}

void JobManager::readSettings(QSettings &settings)
{
  int numJobs = settings.beginReadArray("Jobs");
  for (int i = 0; i < numJobs; ++i) {
    settings.setArrayIndex(i);
    QVariantHash hash = settings.value("hash", QVariantHash ()).toHash();
    Job *job = new Job ();
    job->setFromHash(hash);
    this->insertJob(job);
  }
  settings.endArray();
}

void JobManager::writeSettings(QSettings &settings) const
{
  settings.beginWriteArray("Jobs", m_jobs.size());
  for (int i = 0; i < m_jobs.size(); ++i) {
    settings.setArrayIndex(i);
    settings.setValue("hash", m_jobs.at(i)->hash());
  }
  settings.endArray(); // Jobs
}

Job *JobManager::newJob()
{
  Job *job = new Job();

  emit jobAboutToBeAdded(job);

  this->insertJob(job);
  return job;
}

Job *JobManager::newJob(const QVariantHash &jobState)
{
  Job *job = new Job();
  job->setFromHash(jobState);

  emit jobAboutToBeAdded(job);

  this->insertJob(job);
  return job;
}

void JobManager::removeJob(const Job *job)
{
  if (!job || !m_jobs.contains(job))
    return;

  emit jobAboutToBeRemoved(job);

  IdType moleQueueId = job->moleQueueId();

  m_jobs.removeOne(job);
  m_clientMap.remove(job->clientId());
  m_moleQueueMap.remove(moleQueueId);

  delete const_cast<Job*>(job);

  emit jobRemoved(moleQueueId, job);
}

void JobManager::removeJob(IdType moleQueueId)
{
  const Job *job = this->lookupMoleQueueId(moleQueueId);

  if (job)
    this->removeJob(job);
}

void JobManager::removeJobs(const QList<const Job *> &jobsToRemove)
{
  foreach (const Job* job, jobsToRemove)
    this->removeJob(job);
}

void JobManager::removeJobs(const QList<IdType> &moleQueueIds)
{
  foreach(IdType moleQueueId, moleQueueIds)
    this->removeJob(moleQueueId);
}

const Job *JobManager::lookupClientId(IdType clientId) const
{
  return m_clientMap.value(clientId, NULL);
}

const Job *JobManager::lookupMoleQueueId(IdType moleQueueId) const
{
  return m_moleQueueMap.value(moleQueueId, NULL);
}

QList<const Job *> JobManager::jobsWithJobState(JobState state)
{
  QList<const Job*> result;

  foreach (const Job *job, m_jobs) {
    if (job->jobState() == state)
      result << job;
  }

  return result;
}

void JobManager::jobIdsChanged(const Job *job)
{
  if (!m_jobs.contains(job))
    return;

  if (m_clientMap.value(job->clientId(), NULL) != job) {
    IdType oldClientId = m_clientMap.key(job, 0);
    if (oldClientId != 0)
      m_clientMap.remove(oldClientId);
    m_clientMap.insert(job->clientId(), job);
  }

  if (m_moleQueueMap.value(job->moleQueueId(), NULL) != job) {
    IdType oldMoleQueueId = m_moleQueueMap.key(job, 0);
    if (oldMoleQueueId != 0)
      m_moleQueueMap.remove(oldMoleQueueId);
    m_moleQueueMap.insert(job->moleQueueId(), job);
  }
}

void JobManager::updateJobState(IdType moleQueueId, JobState newState)
{
  const Job *job = this->lookupMoleQueueId(moleQueueId);
  if (!job)
    return;

  const JobState oldState = job->jobState();

  if (oldState == newState)
    return;

  const_cast<Job*>(job)->setJobState(newState);

  emit jobStateChanged(job, oldState, newState);
}

void JobManager::updateQueueId(IdType moleQueueId, IdType queueId)
{
  const Job *job = this->lookupMoleQueueId(moleQueueId);
  if (!job)
    return;

  if (job->queueJobId() == queueId)
    return;

  const_cast<Job*>(job)->setQueueJobId(queueId);

  emit queueIdChanged(job, queueId);
}

void JobManager::insertJob(Job *job)
{
  m_jobs.append(job);
  if (job->clientId() != 0)
    m_clientMap.insert(job->clientId(), job);
  if (job->moleQueueId() != 0)
    m_moleQueueMap.insert(job->moleQueueId(), job);

  emit jobAdded(job);
}

} // end namespace MoleQueue
