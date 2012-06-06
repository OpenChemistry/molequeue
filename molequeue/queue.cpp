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

#include "queue.h"

#include "program.h"
#include "queuemanager.h"
#include "server.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>

// typedef used to work around commas in macro
typedef QMap<MoleQueue::IdType, MoleQueue::IdType> JobIdMapType;
Q_DECLARE_METATYPE(JobIdMapType)

namespace MoleQueue {

Queue::Queue(const QString &queueName, QueueManager *parentManager) :
  QObject(parentManager), m_queueManager(parentManager),
  m_server((m_queueManager) ? m_queueManager->server() : NULL),
  m_name(queueName)
{
  qRegisterMetaType<Program*>("MoleQueue::Program*");
  qRegisterMetaType<const Program*>("const MoleQueue::Program*");
}

Queue::~Queue()
{
  QList<Program*> programList = m_programs.values();
  m_programs.clear();
  qDeleteAll(programList);
}

void Queue::readSettings(QSettings &settings)
{
  m_launchTemplate = settings.value("launchTemplate").toString();
  m_launchScriptName = settings.value("launchScriptName").toString();

  m_jobs = settings.value("jobIdMap").value<JobIdMapType>();

  QStringList progNames = settings.value("programs").toStringList();

  settings.beginGroup("Programs");
  foreach (const QString &progName, progNames) {
    settings.beginGroup(progName);

    Program *program = new Program (this);
    program->setName(progName);
    program->readSettings(settings);

    if (!this->addProgram(program)) {
      qWarning() << Q_FUNC_INFO << "Could not add program" << progName
                 << "to queue" << this->name() << "-- duplicate program name.";
      delete program;
    }

    settings.endGroup(); // progName
  }
  settings.endGroup(); // "Programs"
}

void Queue::writeSettings(QSettings &settings) const
{
  settings.setValue("launchTemplate", m_launchTemplate);
  settings.setValue("launchScriptName", m_launchScriptName);

  settings.setValue("jobIdMap", QVariant::fromValue(m_jobs));

  settings.setValue("programs", this->programNames());

  settings.beginGroup("Programs");
  foreach (const Program *prog, this->programs()) {
    settings.beginGroup(prog->name());
    prog->writeSettings(settings);
    settings.endGroup(); // prog->name()
  }
  settings.endGroup(); // "Programs"
}

QWidget* Queue::settingsWidget() const
{
  return NULL;
}

bool Queue::addProgram(Program *newProgram, bool replace)
{
  // Check for duplicates, unless we are replacing, and return false if found.
  if (m_programs.contains(newProgram->name())) {
    if (replace)
      m_programs.take(newProgram->name())->deleteLater();
    else
      return false;
  }

  m_programs.insert(newProgram->name(), newProgram);

  emit programAdded(newProgram->name(), newProgram);
  return true;
}

bool Queue::removeProgram(Program* programToRemove)
{
  return removeProgram(programToRemove->name());
}

bool Queue::removeProgram(const QString &programName)
{
  if (!m_programs.contains(programName))
    return false;

  Program *program = m_programs.take(programName);

  emit programRemoved(programName, program);
  return true;
}

} // End namespace
