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

#include "job.h"
#include "jobdata.h"
#include "jobmanager.h"

namespace MoleQueue
{

Job::Job(JobData *jobdata)
  : JobReferenceBase(jobdata)
{
}

Job::Job(JobManager *jobManager, IdType mQId)
  : JobReferenceBase(jobManager, mQId)
{
}

Job::Job(const JobReferenceBase &other)
  : JobReferenceBase(other)
{
}

Job::~Job()
{
}

void Job::setFromHash(const QVariantHash &state)
{
  if (warnIfInvalid())
    m_jobData->setFromHash(state);
}

QVariantHash Job::hash() const
{
  if (warnIfInvalid())
    return m_jobData->hash();
  return QVariantHash();
}

void Job::setQueue(const QString &newQueue)
{

  if (warnIfInvalid())
    m_jobData->setQueue(newQueue);
}

QString Job::queue() const
{
  if (warnIfInvalid())
    return m_jobData->queue();
  return QString();
}

void Job::setProgram(const QString &newProgram)
{
  if (warnIfInvalid())
    m_jobData->setProgram(newProgram);
}

QString Job::program() const
{
  if (warnIfInvalid())
    return m_jobData->program();
  return QString();
}

void Job::setJobState(JobState state)
{
  if (warnIfInvalid())
    m_jobData->jobManager()->setJobState(moleQueueId(), state);
}

JobState Job::jobState() const
{
  if (warnIfInvalid())
    return m_jobData->jobState();
  return MoleQueue::Unknown;
}

void Job::setDescription(const QString &newDesc)
{
  if (warnIfInvalid())
    m_jobData->setDescription(newDesc);
}

QString Job::description() const
{
  if (warnIfInvalid())
    return m_jobData->description();
  return QString();
}

void Job::setInputAsPath(const QString &path)
{
  if (warnIfInvalid())
    m_jobData->setInputAsPath(path);
}

QString Job::inputAsPath() const
{
  if (warnIfInvalid())
    return m_jobData->inputAsPath();
  return QString();
}

void Job::setInputAsString(const QString &input)
{
  if (warnIfInvalid())
    m_jobData->setInputAsString(input);
}

QString Job::inputAsString() const
{
  if (warnIfInvalid())
    return m_jobData->inputAsString();
  return QString();
}

void Job::setOutputDirectory(const QString &path)
{
  if (warnIfInvalid())
    m_jobData->setOutputDirectory(path);
}

QString Job::outputDirectory() const
{
  if (warnIfInvalid())
    return m_jobData->outputDirectory();
  return QString();
}

void Job::setLocalWorkingDirectory(const QString &path)
{
  if (warnIfInvalid())
    m_jobData->setLocalWorkingDirectory(path);
}

QString Job::localWorkingDirectory() const
{
  if (warnIfInvalid())
    return m_jobData->localWorkingDirectory();
  return QString();
}

void Job::setCleanRemoteFiles(bool clean)
{
  if (warnIfInvalid())
    m_jobData->setCleanRemoteFiles(clean);
}

bool Job::cleanRemoteFiles() const
{
  if (warnIfInvalid())
    return m_jobData->cleanRemoteFiles();
  return false;
}

void Job::setRetrieveOutput(bool b)
{
  if (warnIfInvalid())
    m_jobData->setRetrieveOutput(b);
}

bool Job::retrieveOutput() const
{
  if (warnIfInvalid())
    return m_jobData->retrieveOutput();
  return false;
}

void Job::setCleanLocalWorkingDirectory(bool b)
{
  if (warnIfInvalid())
    m_jobData->setCleanLocalWorkingDirectory(b);
}

bool Job::cleanLocalWorkingDirectory() const
{
  if (warnIfInvalid())
    return m_jobData->cleanLocalWorkingDirectory();
  return false;
}

void Job::setHideFromGui(bool b)
{
  if (warnIfInvalid())
    m_jobData->setHideFromGui(b);
}

bool Job::hideFromGui() const
{
  if (warnIfInvalid())
    return m_jobData->hideFromGui();
  return false;
}

void Job::setPopupOnStateChange(bool b)
{
  if (warnIfInvalid())
    m_jobData->setPopupOnStateChange(b);
}

bool Job::popupOnStateChange() const
{
  if (warnIfInvalid())
    return m_jobData->popupOnStateChange();
  return false;
}

void Job::setMoleQueueId(IdType id)
{
  if (warnIfInvalid()) {
    m_jobData->setMoleQueueId(id);
    m_jobData->jobManager()->moleQueueIdChanged(*this);
  }
}

IdType Job::moleQueueId() const
{
  if (warnIfInvalid())
    return m_jobData->moleQueueId();
  return InvalidId;
}

void Job::setQueueId(IdType id)
{
  if (warnIfInvalid())
    m_jobData->jobManager()->setJobQueueId(m_jobData->moleQueueId(), id);
}

IdType Job::queueId() const
{
  if (warnIfInvalid())
    return m_jobData->queueId();
  return InvalidId;
}

} // end namespace MoleQueue