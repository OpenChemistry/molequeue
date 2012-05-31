/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "sge.h"

#include "../job.h"
#include "../program.h"
#include "../terminalprocess.h"
#include "../sshcommand.h"

#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

namespace MoleQueue {

QueueSGE::QueueSGE(QueueManager *parentManager) :
  Queue("Remote (SGE)", parentManager)
{
}

QueueSGE::~QueueSGE()
{
}

bool QueueSGE::submitJob(const Job *job)
{
  /// @todo This needs to be rewritten
  Q_UNUSED(job);
  /*
  m_jobs.push_back(job);
  job->setJobState(MoleQueue::LocalQueued);
  emit(jobAdded(job));
  submitJob(m_jobs.size() - 1);
  */
  return true;
}

} // End namespace
