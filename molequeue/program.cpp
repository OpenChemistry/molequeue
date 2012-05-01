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

#include "program.h"

#include "job.h"
#include "queue.h"

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>

namespace MoleQueue {

Program::Program(Queue *queue) :
  m_runDirect(true),
  m_delimiter("$$"),
  m_queue(queue)
{
}

Program::~Program()
{
}

Program::Program(const Program &other)
{
  m_runDirect = other.m_runDirect;
  m_runTemplate = other.m_runTemplate;
  m_delimiter = other.m_delimiter;
  m_queue = other.m_queue;
  m_title = other.m_title;
  m_name = other.m_name;
}

Job* Program::createJob() const
{
  Job *job = new Job(this);

  job->setName(m_name);
  job->setTitle(m_title);

  return job;
}

QString Program::queueName() const
{
  if (m_queue)
    return m_queue->name();
  else
    return "None";
}

} // End namespace
