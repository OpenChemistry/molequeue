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

#include "jobactionfactory.h"

#include "job.h"

namespace MoleQueue
{

JobActionFactory::JobActionFactory()
 : QObject(), m_attemptedJobAdditions(0), m_isMultiJob(false), m_server(NULL)
{
}

JobActionFactory::JobActionFactory(const JobActionFactory &other)
  : QObject(),
    m_attemptedJobAdditions(other.m_attemptedJobAdditions),
    m_isMultiJob(other.m_isMultiJob),
    m_server(other.m_server),
    m_jobs(other.m_jobs),
    m_flags(other.m_flags)
{
}

JobActionFactory::~JobActionFactory()
{
}

JobActionFactory &JobActionFactory::operator =(const JobActionFactory &other)
{
  m_attemptedJobAdditions = other.m_attemptedJobAdditions;
  m_isMultiJob = other.m_isMultiJob;
  m_server = other.m_server;
  m_jobs = other.m_jobs;
  m_flags = other.m_flags;
  return *this;
}

void JobActionFactory::readSettings(QSettings &settings)
{
  m_isMultiJob = settings.value("isMultiJob").toBool();
  m_flags = static_cast<Flags>(settings.value("flags").toInt());
}

void JobActionFactory::writeSettings(QSettings &settings) const
{
  settings.setValue("isMultiJob", m_isMultiJob);
  settings.setValue("flags", static_cast<int>(m_flags));
}

void JobActionFactory::clearJobs()
{
  m_attemptedJobAdditions = 0;
  m_jobs.clear();
}

bool JobActionFactory::isMultiJob() const
{
  return m_isMultiJob;
}

bool JobActionFactory::addJobIfValid(const Job &job)
{
  ++m_attemptedJobAdditions;
  bool result = this->isValidForJob(job);
  if (result)
    m_jobs.append(job);
  return result;
}

bool JobActionFactory::hasValidActions() const
{
  return static_cast<bool>(m_jobs.size());
}

JobActionFactory::Flags JobActionFactory::flags() const
{
  return m_flags;
}

void JobActionFactory::setFlags(JobActionFactory::Flags f)
{
  m_flags = f;
}

}
