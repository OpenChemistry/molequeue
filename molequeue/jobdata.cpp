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

#include "jobdata.h"
#include "jobmanager.h"

namespace MoleQueue
{

JobData::JobData(JobManager *parentManager)
  : QObject(parentManager),
    m_jobManager(parentManager),
    m_jobState(MoleQueue::None),
    m_cleanRemoteFiles(false),
    m_retrieveOutput(true),
    m_cleanLocalWorkingDirectory(false),
    m_hideFromGui(false),
    m_popupOnStateChange(true),
    m_moleQueueId(InvalidId),
    m_queueId(InvalidId)
{
}

JobData::JobData(const MoleQueue::JobData &other)
  : QObject(other.jobManager()),
    m_jobManager(other.m_jobManager),
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
    m_hideFromGui(other.m_hideFromGui),
    m_popupOnStateChange(other.m_popupOnStateChange),
    m_moleQueueId(other.m_moleQueueId),
    m_queueId(other.m_queueId)
{
}

QVariantHash JobData::hash() const
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
  state.insert("hideFromGui", m_hideFromGui);
  state.insert("popupOnStateChange", m_popupOnStateChange);
  state.insert("moleQueueId", m_moleQueueId);
  state.insert("queueId", m_queueId);

  return state;
}

void JobData::setFromHash(const QVariantHash &state)
{
  /// @todo only set these if they exist in the hash
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
  m_hideFromGui = state.value("hideFromGui", false).toBool();
  m_popupOnStateChange = state.value("popupOnStateChange", true).toBool();
  m_moleQueueId = static_cast<IdType>(
        state.value("moleQueueId", 0).toUInt());
  m_queueId = static_cast<IdType>(
        state.value("queueId", 0).toUInt());
}

} // end namespace MoleQueue
