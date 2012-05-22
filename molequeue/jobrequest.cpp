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

JobRequest::JobRequest(MoleQueue::Client *parent)
  : m_client(parent),
    m_jobState(MoleQueue::None),
    m_cleanRemoteFiles(false),
    m_retrieveOutput(true),
    m_cleanLocalWorkingDirectory(false),
    m_hideFromQueue(false),
    m_popupOnStateChange(true),
    m_molequeueId(0),
    m_queueJobId(0),
    m_clientId(0)
{
}

JobRequest::JobRequest(const MoleQueue::JobRequest &other)
  : m_client(other.m_client),
    m_queue(other.m_queue),
    m_program(other.m_program),
    m_jobState(other.m_jobState),
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
    m_molequeueId(other.m_molequeueId),
    m_queueJobId(other.m_queueJobId),
    m_clientId(other.m_clientId)
{
}

QVariantHash JobRequest::hash() const
{
  QVariantHash state;

  state.insert("queue", m_queue);
  state.insert("program", m_program);
  state.insert("jobState", m_jobState);
  state.insert("description", m_description);
  state.insert("inputAsPath", m_inputAsPath);
  state.insert("inputAsString", m_inputAsString);
  state.insert("outputDirectory", m_outputDirectory);
  state.insert("localWorkingDirectory", m_localWorkingDirectory);
  state.insert("cleanRemoteFiles", m_cleanRemoteFiles);
  state.insert("retrieveOutput", m_retrieveOutput);
  state.insert("cleanLocalWorkingDirectory", m_cleanLocalWorkingDirectory);
  state.insert("hideFromQueue", m_hideFromQueue);
  state.insert("popupOnStateChange", m_popupOnStateChange);
  state.insert("molequeueId", m_molequeueId);
  state.insert("queueJobId", m_queueJobId);
  state.insert("clientId", m_clientId);

  return state;
}

void JobRequest::setFromHash(const QVariantHash &state)
{
  m_queue = state.value("queue", "").toString();
  m_program = state.value("program", "").toString();
  m_description = state.value("description", "").toString();
  m_jobState = static_cast<JobState>(
        state.value("jobState", MoleQueue::None).toInt());
  m_inputAsPath = state.value("inputAsPath", "").toString();
  m_inputAsString = state.value("inputAsString", "").toString();
  m_outputDirectory = state.value("outputDirectory", "").toString();
  m_localWorkingDirectory = state.value("localWorkingDirectory", "").toString();
  m_cleanRemoteFiles = state.value("cleanRemoteFiles", false).toBool();
  m_retrieveOutput = state.value("retrieveOutput", true).toBool();
  m_cleanLocalWorkingDirectory =
      state.value("cleanLocalWorkingDirectory", false).toBool();
  m_hideFromQueue = state.value("hideFromQueue", false).toBool();
  m_popupOnStateChange = state.value("popupOnStateChange", true).toBool();
  m_molequeueId = static_cast<IdType>(
        state.value("molequeueId", 0).toUInt());
  m_queueJobId = static_cast<IdType>(
        state.value("queueJobId", 0).toUInt());
  m_clientId = static_cast<IdType>(
        state.value("clientId", 0).toUInt());
}

} // end namespace MoleQueue
