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

#include "jobrequest.h"

namespace MoleQueue
{

JobRequest::JobRequest(MoleQueue::MoleQueueClient *parent)
  : m_client(parent),
    m_cleanRemoteFiles(false),
    m_retrieveOutput(true),
    m_cleanLocalWorkingDirectory(false),
    m_hideFromQueue(false),
    m_popupOnStateChange(true),
    m_molequeueId(0),
    m_queueJobId(0)
{
}

JobRequest::JobRequest(const MoleQueue::JobRequest &other)
  : m_client(other.m_client),
    m_queue(other.m_queue),
    m_program(other.m_program),
    m_description(other.m_description),
    m_inputAsPath(other.m_inputAsPath),
    m_inputAsString(other.m_inputAsString),
    m_outputDirectory(other.m_outputDirectory),
    m_localWorkingDirectory(other.m_localWorkingDirectory),
    m_cleanRemoteFiles(other.m_cleanRemoteFiles),
    m_retrieveOutput(other.m_retrieveOutput),
    m_cleanLocalWorkingDirectory(other.m_cleanLocalWorkingDirectory),
    m_hideFromQueue(other.m_hideFromQueue),
    m_popupOnStateChange(other.m_popupOnStateChange),
    m_molequeueId(0),
    m_queueJobId(0)
{
}

} // end namespace MoleQueue
