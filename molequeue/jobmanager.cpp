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
  qRegisterMetaType<Job*>("Job*");
  qRegisterMetaType<const Job*>("const Job*");
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

Job *JobManager::lookupClientId(IdType clientId) const
{
  return m_clientMap.value(clientId, NULL);
}

Job *JobManager::lookupMoleQueueId(IdType moleQueueId) const
{
  return m_moleQueueMap.value(moleQueueId, NULL);
}

void JobManager::insertJob(Job *job)
{
  m_jobs.append(job);
  m_clientMap.insert(job->clientId(), job);
  m_moleQueueMap.insert(job->moleQueueId(), job);
}

} // end namespace MoleQueue
