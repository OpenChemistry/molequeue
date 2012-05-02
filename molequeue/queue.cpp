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

#include <QtCore/QSettings>

namespace MoleQueue {

Queue::Queue(const QString &queueName, QObject *parentObject) :
  QObject(parentObject), m_name(queueName), m_jobIndexOffset(0)
{
}

Queue::~Queue()
{

}

void Queue::readSettings(const QSettings &settings)
{
  m_jobIndexOffset = settings.value("jobIndexOffset", 0).toUInt();
}

void Queue::writeSettings(QSettings &settings) const
{
  settings.setValue("jobIndexOffset", m_jobIndexOffset + m_jobs.size());
}

QWidget* Queue::settingsWidget() const
{
  return 0;
}

bool Queue::addProgram(Program *newProgram, bool replace)
{
  // Check for duplicates, unless we are replacing, and return false if found.
  if (!replace && m_programs.contains(newProgram->name()))
    return false;

  m_programs[newProgram->name()] = newProgram;
  return true;
}

bool Queue::removeProgram(Program* programToRemove)
{
  return removeProgram(programToRemove->name());
}

bool Queue::removeProgram(const QString &programName)
{
  return m_programs.remove(programName) >= 1 ? true : false;
}

Program* Queue::program(const QString &programName)
{
  return m_programs.value(programName, 0);
}

void Queue::clearPrograms()
{
  m_programs.clear();
}

QStringList Queue::programs() const
{
  QStringList programList;
  foreach(const Program *prog, m_programs)
    programList << prog->name();
  return programList;
}

bool Queue::submit(Job *job)
{
  Q_UNUSED(job);

  return false;
}

} // End namespace
