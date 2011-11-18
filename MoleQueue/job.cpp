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

#include "job.h"

#include "program.h"

namespace MoleQueue {

Job::Job(const Program *program)
  : QObject(),
    m_program(program)
{
}

Job::~Job()
{
}

void Job::setName(const QString &name)
{
  m_name = name;
}

QString Job::name() const
{
  return m_name;
}

void Job::setTitle(const QString &title)
{
  m_title = title;
}

QString Job::title() const
{
  return m_title;
}

const Program* Job::program() const
{
  return m_program;
}

const Queue* Job::queue() const
{
  return m_program->queue();
}

} // end MoleQueue namespace
