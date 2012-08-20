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
  : m_jobManager(parentManager),
    m_jobState(MoleQueue::None),
    m_cleanRemoteFiles(false),
    m_retrieveOutput(true),
    m_cleanLocalWorkingDirectory(false),
    m_hideFromGui(false),
    m_popupOnStateChange(false),
    m_numberOfCores(DEFAULT_NUM_CORES),
    m_maxWallTime(-1), // use default queue time
    m_moleQueueId(InvalidId),
    m_queueId(InvalidId)
{
}

JobData::JobData(const MoleQueue::JobData &other)
  : m_jobManager(other.m_jobManager),
    m_queue(other.m_queue),
    m_program(other.m_program),
    m_jobState(other.m_jobState),
    m_description(other.m_description),
    m_inputFile(other.m_inputFile),
    m_additionalInputFiles(other.m_additionalInputFiles),
    m_outputDirectory(other.m_outputDirectory),
    m_localWorkingDirectory(other.m_localWorkingDirectory),
    m_cleanRemoteFiles(other.m_cleanRemoteFiles),
    m_retrieveOutput(other.m_retrieveOutput),
    m_cleanLocalWorkingDirectory(other.m_cleanLocalWorkingDirectory),
    m_hideFromGui(other.m_hideFromGui),
    m_popupOnStateChange(other.m_popupOnStateChange),
    m_numberOfCores(other.m_numberOfCores),
    m_maxWallTime(other.m_maxWallTime),
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
  state.insert("inputFile", m_inputFile.asVariantHash());
  if (!m_additionalInputFiles.isEmpty()) {
    QList<QVariant> additionalFiles;
    foreach (const FileSpec &spec, m_additionalInputFiles) {
      additionalFiles.append(spec.asVariantHash());
    }
    state.insert("additionalInputFiles", additionalFiles);
  }
  state.insert("outputDirectory", m_outputDirectory);
  state.insert("localWorkingDirectory", m_localWorkingDirectory);
  state.insert("cleanRemoteFiles", m_cleanRemoteFiles);
  state.insert("retrieveOutput", m_retrieveOutput);
  state.insert("cleanLocalWorkingDirectory", m_cleanLocalWorkingDirectory);
  state.insert("hideFromGui", m_hideFromGui);
  state.insert("popupOnStateChange", m_popupOnStateChange);
  state.insert("numberOfCores", m_numberOfCores);
  state.insert("maxWallTime", m_maxWallTime);
  state.insert("moleQueueId", m_moleQueueId);
  state.insert("queueId", m_queueId);

  return state;
}

void JobData::setFromHash(const QVariantHash &state)
{
  if (state.contains("queue"))
    m_queue = state.value("queue").toString();
  if (state.contains("program"))
    m_program = state.value("program").toString();
  if (state.contains("description"))
    m_description = state.value("description").toString();
  if (state.contains("jobState"))
    m_jobState = static_cast<JobState>(state.value("jobState").toInt());
  if (state.contains("inputFile"))
    m_inputFile = FileSpec(state.value("inputFile").toHash());
  m_additionalInputFiles.clear();
  if (state.contains("additionalInputFiles")) {
    foreach(const QVariant &variantHash,
            state.value("additionalInputFiles").toList()) {
      m_additionalInputFiles.append(FileSpec(variantHash.toHash()));
    }
  }
  if (state.contains("outputDirectory"))
    m_outputDirectory = state.value("outputDirectory").toString();
  if (state.contains("localWorkingDirectory"))
    m_localWorkingDirectory = state.value("localWorkingDirectory").toString();
  if (state.contains("cleanRemoteFiles"))
    m_cleanRemoteFiles = state.value("cleanRemoteFiles").toBool();
  if (state.contains("retrieveOutput"))
    m_retrieveOutput = state.value("retrieveOutput").toBool();
  if (state.contains("cleanLocalWorkingDirectory")) {
    m_cleanLocalWorkingDirectory =
        state.value("cleanLocalWorkingDirectory").toBool();
  }
  if (state.contains("hideFromGui"))
    m_hideFromGui = state.value("hideFromGui").toBool();
  if (state.contains("popupOnStateChange"))
    m_popupOnStateChange = state.value("popupOnStateChange").toBool();
  if (state.contains("numberOfCores"))
    m_numberOfCores = state.value("numberOfCores").toInt();
  if (state.contains("maxWallTime"))
    m_maxWallTime = state.value("maxWallTime").toInt();
  if (state.contains("moleQueueId"))
    m_moleQueueId = static_cast<IdType>(state.value("moleQueueId").toUInt());
  if (state.contains("queueId"))
    m_queueId = static_cast<IdType>(state.value("queueId").toUInt());
}

} // end namespace MoleQueue
