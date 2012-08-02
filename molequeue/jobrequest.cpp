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

#include "jobdata.h"

namespace MoleQueue
{

JobRequest::JobRequest(JobData *jobdata)
  : JobReferenceBase(jobdata)
{
}

JobRequest::JobRequest(JobManager *jobManager, IdType mQId)
  : JobReferenceBase(jobManager, mQId)
{
}

JobRequest::JobRequest(const JobReferenceBase &other)
  : JobReferenceBase(other)
{
}

JobRequest::~JobRequest()
{
}

QVariantHash JobRequest::hash() const
{
  if (warnIfInvalid())
    return m_jobData->hash();
  return QVariantHash();
}

void JobRequest::setQueue(const QString &newQueue)
{

  if (warnIfInvalid())
    m_jobData->setQueue(newQueue);
}

QString JobRequest::queue() const
{
  if (warnIfInvalid())
    return m_jobData->queue();
  return QString();
}

void JobRequest::setProgram(const QString &newProgram)
{
  if (warnIfInvalid())
    m_jobData->setProgram(newProgram);
}

QString JobRequest::program() const
{
  if (warnIfInvalid())
    return m_jobData->program();
  return QString();
}

JobState JobRequest::jobState() const
{
  if (warnIfInvalid())
    return m_jobData->jobState();
  return MoleQueue::Unknown;
}

void JobRequest::setDescription(const QString &newDesc)
{
  if (warnIfInvalid())
    m_jobData->setDescription(newDesc);
}

QString JobRequest::description() const
{
  if (warnIfInvalid())
    return m_jobData->description();
  return QString();
}

void JobRequest::setInputAsPath(const QString &path)
{
  if (warnIfInvalid())
    m_jobData->setInputAsPath(path);
}

QString JobRequest::inputAsPath() const
{
  if (warnIfInvalid())
    return m_jobData->inputAsPath();
  return QString();
}

void JobRequest::setInputAsString(const QString &input)
{
  if (warnIfInvalid())
    m_jobData->setInputAsString(input);
}

QString JobRequest::inputAsString() const
{
  if (warnIfInvalid())
    return m_jobData->inputAsString();
  return QString();
}

void JobRequest::setOutputDirectory(const QString &path)
{
  if (warnIfInvalid())
    m_jobData->setOutputDirectory(path);
}

QString JobRequest::outputDirectory() const
{
  if (warnIfInvalid())
    return m_jobData->outputDirectory();
  return QString();
}

QString JobRequest::localWorkingDirectory() const
{
  if (warnIfInvalid())
    return m_jobData->localWorkingDirectory();
  return QString();
}

void JobRequest::setCleanRemoteFiles(bool clean)
{
  if (warnIfInvalid())
    m_jobData->setCleanRemoteFiles(clean);
}

bool JobRequest::cleanRemoteFiles() const
{
  if (warnIfInvalid())
    return m_jobData->cleanRemoteFiles();
  return false;
}

void JobRequest::setRetrieveOutput(bool b)
{
  if (warnIfInvalid())
    m_jobData->setRetrieveOutput(b);
}

bool JobRequest::retrieveOutput() const
{
  if (warnIfInvalid())
    return m_jobData->retrieveOutput();
  return false;
}

void JobRequest::setCleanLocalWorkingDirectory(bool b)
{
  if (warnIfInvalid())
    m_jobData->setCleanLocalWorkingDirectory(b);
}

bool JobRequest::cleanLocalWorkingDirectory() const
{
  if (warnIfInvalid())
    return m_jobData->cleanLocalWorkingDirectory();
  return false;
}

void JobRequest::setHideFromGui(bool b)
{
  if (warnIfInvalid())
    m_jobData->setHideFromGui(b);
}

bool JobRequest::hideFromGui() const
{
  if (warnIfInvalid())
    return m_jobData->hideFromGui();
  return false;
}

void JobRequest::setPopupOnStateChange(bool b)
{
  if (warnIfInvalid())
    m_jobData->setPopupOnStateChange(b);
}

bool JobRequest::popupOnStateChange() const
{
  if (warnIfInvalid())
    return m_jobData->popupOnStateChange();
  return false;
}

void JobRequest::setNumberOfCores(int num)
{
  if (warnIfInvalid())
    m_jobData->setNumberOfCores(num);
}

int JobRequest::numberOfCores() const
{
  if (warnIfInvalid())
    return m_jobData->numberOfCores();
  return false;
}

void JobRequest::setMaxWallTime(int minutes)
{
  if (warnIfInvalid())
    m_jobData->setMaxWallTime(minutes);
}

int JobRequest::maxWallTime() const
{
  if (warnIfInvalid())
    return m_jobData->maxWallTime();
  return -1;
}

IdType JobRequest::moleQueueId() const
{
  if (warnIfInvalid())
    return m_jobData->moleQueueId();
  return InvalidId;
}

IdType JobRequest::queueId() const
{
  if (warnIfInvalid())
    return m_jobData->queueId();
  return InvalidId;
}

} // namespace MoleQueue
