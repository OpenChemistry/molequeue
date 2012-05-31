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

const Job *JobManager::lookupClientId(IdType clientId) const
{
  return m_clientMap.value(clientId, NULL);
}

const Job *JobManager::lookupMoleQueueId(IdType moleQueueId) const
{
  return m_moleQueueMap.value(moleQueueId, NULL);
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

  this->mut(job)->setJobState(newState);

  emit jobStateChanged(job, oldState, newState);
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
