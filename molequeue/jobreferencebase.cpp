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

#include "jobreferencebase.h"

#include "jobdata.h"
#include "jobmanager.h"

namespace MoleQueue {

JobReferenceBase::JobReferenceBase(JobData *jobdata)
  : m_jobData(jobdata),
    m_moleQueueId( Q_LIKELY(jobdata != NULL) ? jobdata->moleQueueId() :
                                               MoleQueue::InvalidId )

{
}

JobReferenceBase::JobReferenceBase(JobManager *jobManager, IdType moleQueueId)
  : m_jobData(jobManager->lookupJobDataByMoleQueueId(moleQueueId)),
    m_moleQueueId(Q_LIKELY(m_jobData != NULL) ? m_jobData->moleQueueId() :
                                                MoleQueue::InvalidId)
{
}

JobReferenceBase::~JobReferenceBase()
{
}

bool JobReferenceBase::operator ==(const JobReferenceBase &other) const
{
  return isValid() && other.isValid() &&
      m_jobData.data() == other.m_jobData.data();
}

JobData * JobReferenceBase::jobData() const
{
  return m_jobData.data();
}

} // namespace MoleQueue
