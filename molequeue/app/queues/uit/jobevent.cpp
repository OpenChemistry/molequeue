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

#include "queues/uit/jobevent.h"

namespace MoleQueue {
namespace Uit {

JobEvent::JobEvent()
  : m_eventTime(-1), m_jobID(-1)
{

}

JobEvent::JobEvent(const JobEvent &other)
  :  m_acctHost(other.acctHost()), m_eventType(other.eventType()),
     m_eventTime(other.eventTime()), m_jobID(other.jobId()),
     m_jobQueue(other.jobQueue()), m_jobStatus(other.jobStatus()),
     m_jobStatusText(other.jobStatusText())
{

}


JobEvent &JobEvent::operator=(
  const JobEvent &other)
{
  if (this != &other) {
    m_acctHost = other.acctHost();
    m_eventType = other.eventType();
    m_eventTime = other.eventTime();
    m_jobID = other.jobId();
    m_jobQueue = other.jobQueue();
    m_jobStatus = other.jobStatus();
    m_jobStatusText = other.jobStatusText();
  }

  return *this;
}

} /* namespace Uit */
} /* namespace MoleQueue */
