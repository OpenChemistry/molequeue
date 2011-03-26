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

#include "queue.h"

namespace MoleQueue {

Queue::Queue(QObject *parent) :
  QObject(parent), m_name("Undefined")
{
}

Queue::~Queue()
{

}

bool Queue::addProgram(const Program &program, bool replace)
{
  // Check for duplicates, unless we are replacing, and return false if found.
  if (!replace && m_programs.contains(program.name()))
    return false;

  m_programs[program.name()] = program;
  return true;
}

bool Queue::removeProgram(const Program &program)
{
  return removeProgram(program.name());
}

bool Queue::removeProgram(const QString &name)
{
  return m_programs.remove(name) >= 1 ? true : false;
}

Program Queue::program(const QString &name)
{
  if (m_programs.contains(name))
    return m_programs[name];
  else
    return Program(); // FIXME: Set as invalid if required.
}

void Queue::clearPrograms()
{
  m_programs.clear();
}

QStringList Queue::programs() const
{
  QStringList programs;
  foreach(const Program &prog, m_programs)
    programs << prog.name();
  return programs;
}

bool Queue::submit(const Program &job)
{

}

} // End namespace
