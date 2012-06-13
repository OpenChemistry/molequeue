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

#include "queue.h"
#include "queues/remote.h"
#include "queuemanager.h"
#include "server.h"

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>

namespace MoleQueue {

Program::Program(Queue *parentQueue) :
  Object(parentQueue),
  m_queue(parentQueue),
  m_queueManager((m_queue) ? m_queue->queueManager() : NULL),
  m_server((m_queueManager) ? m_queueManager->server() : NULL),
  m_name("Program"),
  m_executable("program"),
  m_useExecutablePath(false),
  m_executablePath(""),
  m_arguments(),
  m_inputFilename("job.inp"),
  m_outputFilename("job.out"),
  m_launchSyntax(REDIRECT),
  m_customLaunchTemplate("")
{
}

Program::Program(const Program &other)
  : Object(other.parent()),
    m_queue(other.m_queue),
    m_name(other.m_name),
    m_executable(other.m_executable),
    m_useExecutablePath(other.m_useExecutablePath),
    m_executablePath(other.m_executablePath),
    m_arguments(other.m_arguments),
    m_inputFilename(other.m_inputFilename),
    m_outputFilename(other.m_outputFilename),
    m_launchSyntax(other.m_launchSyntax),
    m_customLaunchTemplate(other.m_customLaunchTemplate)
{
}

Program::~Program()
{
}

QString Program::queueName() const
{
  if (m_queue)
    return m_queue->name();
  else
    return "None";
}

void Program::readSettings(QSettings &settings)
{
  m_name                 = settings.value("name").toString();
  m_executable           = settings.value("executable").toString();
  m_useExecutablePath    = settings.value("useExecutablePath").toBool();
  m_arguments            = settings.value("arguments").toString();
  m_executablePath       = settings.value("executablePath").toString();
  m_inputFilename        = settings.value("inputFilename").toString();
  m_outputFilename       = settings.value("outputFilename").toString();
  m_customLaunchTemplate = settings.value("customLaunchTemplate").toString();
  m_launchSyntax         = static_cast<LaunchSyntax>(
        settings.value("launchSyntax").toInt());
}

void Program::writeSettings(QSettings &settings) const
{
  settings.setValue("name", m_name);
  settings.setValue("executable", m_executable);
  settings.setValue("useExecutablePath", m_useExecutablePath);
  settings.setValue("executablePath", m_executablePath);
  settings.setValue("arguments", m_arguments);
  settings.setValue("inputFilename", m_inputFilename);
  settings.setValue("outputFilename", m_outputFilename);
  settings.setValue("customLaunchTemplate", m_customLaunchTemplate);
  settings.setValue("launchSyntax", static_cast<int>(m_launchSyntax));
}

QString Program::launchTemplate() const
{
  if (m_launchSyntax == CUSTOM)
    return m_customLaunchTemplate;

  QString result = m_queue ? m_queue->launchTemplate()
                           : QString("$$programExecution$$");
  if (result.contains("$$programExecution$$")) {
    const QString progExec = Program::generateFormattedExecutionString(
          m_executable, m_arguments, m_inputFilename, m_outputFilename,
          m_executablePath, m_useExecutablePath, m_launchSyntax);
    result.replace("$$programExecution$$", progExec);
  }
  if (QueueRemote *remoteQueue = qobject_cast<QueueRemote*>(m_queue)) {
    if (result.contains("$$remoteWorkingDir$$")) {
      const QString remoteWorkingDir = QString("%1/%2/")
          .arg(remoteQueue->workingDirectoryBase())
          .arg("$$moleQueueId$$");
      result.replace("$$remoteWorkingDir$$", remoteWorkingDir);
    }
  }

  return result;
}

QString Program::generateFormattedExecutionString(
    const QString &executableName_, const QString &arguments_,
    const QString &inputFilename_, const QString &outputFilename_,
    const QString &executablePath_,
    bool useExecutablePath_, Program::LaunchSyntax syntax_)
{
  if (syntax_ == Program::CUSTOM) {
    return "";
  }

  QString execStr;

  QString executable =
      ((useExecutablePath_) ? executablePath_ + "/" : QString())
      + executableName_
      + ((arguments_.isEmpty()) ? QString() : " " + arguments_);

  switch (syntax_) {
  case Program::PLAIN:
    execStr += executable;
    break;
  case Program::INPUT_ARG:
    execStr += QString("%1 %2\n").arg(executable).arg(inputFilename_);
    break;
  case Program::INPUT_ARG_NO_EXT:
  {
    execStr += QString("%1 %2\n").arg(executable)
        .arg(Program::chopExtension(inputFilename_));
  }
    break;
  case Program::REDIRECT:
    execStr += QString("%1 < %2 > %3\n")
        .arg(executable).arg(inputFilename_)
        .arg(outputFilename_);
    break;
  case Program::INPUT_ARG_OUTPUT_REDIRECT:
    execStr += QString("%1 %2 > %3\n")
        .arg(executable).arg(inputFilename_)
        .arg(outputFilename_);
    break;
  case Program::CUSTOM:
    // Should be handled as a special case earlier.
    execStr = tr("Internal MoleQueue error: Custom syntax type not handled.\n");
    break;
  default:
    execStr = tr("Internal MoleQueue error: Unrecognized syntax type.\n");
    break;
  }

  return execStr;
}

} // End namespace
