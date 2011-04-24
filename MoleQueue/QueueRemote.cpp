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

#include "QueueRemote.h"

namespace MoleQueue {

QueueRemote::QueueRemote(QObject *parent) :
  Queue("Remote", parent)
{
}

QueueRemote::~QueueRemote()
{
}

bool QueueRemote::submit(const Program &job)
{
  m_jobs.push_back(job);
  m_jobs.back().setStatus(Program::QUEUED);
  return true;
}

} // End namespace
