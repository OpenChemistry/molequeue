/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "program.h"

#include "logger.h"
#include "queue.h"
#include "queues/remote.h"
#include "queuemanager.h"
#include "server.h"

#include <json/json.h>

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>

namespace MoleQueue {

Program::Program(Queue *parentQueue) :
  QObject(parentQueue),
  m_queue(parentQueue),
  m_queueManager((m_queue) ? m_queue->queueManager() : NULL),
  m_server((m_queueManager) ? m_queueManager->server() : NULL),
  m_name("Program"),
  m_executable("program"),
  m_useExecutablePath(false),
  m_executablePath(""),
  m_arguments(),
  m_outputFilename("$$inputFileBaseName$$.out"),
  m_launchSyntax(REDIRECT),
  m_customLaunchTemplate("")
{
}

Program::Program(const Program &other)
  : QObject(other.parent()),
    m_queue(other.m_queue),
    m_name(other.m_name),
    m_executable(other.m_executable),
    m_useExecutablePath(other.m_useExecutablePath),
    m_executablePath(other.m_executablePath),
    m_arguments(other.m_arguments),
    m_outputFilename(other.m_outputFilename),
    m_launchSyntax(other.m_launchSyntax),
    m_customLaunchTemplate(other.m_customLaunchTemplate)
{
}

Program::~Program()
{
}

Program &Program::operator=(const Program &other)
{
  m_queue = other.m_queue;
  m_name = other.m_name;
  m_executable = other.m_executable;
  m_useExecutablePath = other.m_useExecutablePath;
  m_executablePath = other.m_executablePath;
  m_arguments = other.m_arguments;
  m_outputFilename = other.m_outputFilename;
  m_launchSyntax = other.m_launchSyntax;
  m_customLaunchTemplate = other.m_customLaunchTemplate;
  return *this;
}

QString Program::queueName() const
{
  if (m_queue)
    return m_queue->name();
  else
    return "None";
}

bool Program::importSettings(const QString &fileName)
{
  if (!QFile::exists(fileName))
    return false;

  QFile stateFile(fileName);
  if (!stateFile.open(QFile::ReadOnly | QFile::Text)) {
    Logger::logError(tr("Cannot read program information from %1.")
                     .arg(fileName));
    return false;
  }

  // Try to read existing data in
  Json::Value root;
  Json::Reader reader;
  QByteArray inputText = stateFile.readAll();
  if (!reader.parse(inputText.begin(), inputText.end(), root, false)) {
    Logger::logError(tr("Cannot parse program state from %1:\n%2")
                     .arg(fileName).arg(inputText.data()));
    stateFile.close();
    return false;
  }

  if (!root.isObject()) {
    Logger::logError(tr("Error reading program state from %1: "
                        "root is not an object!\n%2")
                     .arg(fileName)
                     .arg(inputText.data()));
    stateFile.close();
    return false;
  }

  return readJsonSettings(root, true);
}

bool Program::exportSettings(const QString &fileName) const
{
  QFile stateFile(fileName);
  if (!stateFile.open(QFile::ReadWrite | QFile::Text | QFile::Truncate)) {
    Logger::logError(tr("Cannot save program information for %1 in queue %2: "
                        "Cannot open file %3.").arg(name())
                     .arg(m_queue->name()).arg(fileName));
    return false;
  }

  Json::Value root(Json::objectValue);
  if (!this->writeJsonSettings(root, true)) {
    stateFile.close();
    return false;
  }

  if (!root.isObject()) {
    Logger::logError(tr("Cannot save program information for %1 in queue %2, "
                        "file %3\nRoot is not an object!").arg(name())
                     .arg(m_queue->name()).arg(fileName));
    stateFile.close();
    return false;
  }

  // Write the data back out:
  std::string outputText = root.toStyledString();
  stateFile.write(QByteArray(outputText.c_str()));
  stateFile.close();

  return true;
}

bool Program::writeJsonSettings(Json::Value &root, bool exportOnly) const
{
  root["executable"] = m_executable.toStdString();
  root["arguments"] = m_arguments.toStdString();
  root["outputFilename"] = m_outputFilename.toStdString();
  root["customLaunchTemplate"] = m_customLaunchTemplate.toStdString();
  root["launchSyntax"] = static_cast<int>(m_launchSyntax);

  if (!exportOnly) {
    root["useExecutablePath"] = m_useExecutablePath;
    root["executablePath"] = m_executablePath.toStdString();
  }

  return true;
}

bool Program::readJsonSettings(const Json::Value &root, bool importOnly)
{
  // Validate JSON
  if (!root.isObject() ||
      !root["executable"].isString() ||
      !root["arguments"].isString() ||
      !root["inputFilename"].isString() ||
      !root["outputFilename"].isString() ||
      !root["customLaunchTemplate"].isString() ||
      !root["launchSyntax"].isIntegral() ||
      (!importOnly && (!root["useExecutablePath"].isBool() ||
                      !root["executablePath"].isString()))) {
    Logger::logError(tr("Error reading program config: Invalid format:\n%1")
                     .arg(QString(root.toStyledString().c_str())));
    return false;
  }

  m_executable = QString(root["executable"].asCString());
  m_arguments = QString(root["arguments"].asCString());
  m_outputFilename = QString(root["outputFilename"].asCString());
  m_customLaunchTemplate = QString(root["customLaunchTemplate"].asCString());
  m_launchSyntax = static_cast<LaunchSyntax>(root["launchSyntax"].asInt());

  if (!importOnly) {
    m_useExecutablePath = root["useExecutablePath"].asBool();
    m_executablePath = QString(root["executablePath"].asCString());
  }
  return true;
}

QString Program::launchTemplate() const
{
  if (m_launchSyntax == CUSTOM)
    return m_customLaunchTemplate;

  QString result = m_queue ? m_queue->launchTemplate()
                           : QString("$$programExecution$$");
  if (result.contains("$$programExecution$$")) {
    const QString progExec = Program::generateFormattedExecutionString(
          m_executable, m_arguments, m_outputFilename, m_executablePath,
          m_useExecutablePath, m_launchSyntax);
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
    const QString &outputFilename_, const QString &executablePath_,
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
    execStr += QString("%1 $$inputFileName$$\n").arg(executable);
    break;
  case Program::INPUT_ARG_NO_EXT:
  {
    execStr += QString("%1 $$inputFileBaseName\n").arg(executable);
  }
    break;
  case Program::REDIRECT:
    execStr += QString("%1 < $$inputFileName$$ > %3\n")
        .arg(executable).arg(outputFilename_);
    break;
  case Program::INPUT_ARG_OUTPUT_REDIRECT:
    execStr += QString("%1 $$inputFileName$$ > %3\n")
        .arg(executable).arg(outputFilename_);
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
