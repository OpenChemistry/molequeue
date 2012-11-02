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

#include "filespecification.h"
#include "filesystemtools.h"
#include "job.h"
#include "jobmanager.h"
#include "logentry.h"
#include "logger.h"
#include "program.h"
#include "queues/local.h"
#include "queues/remote.h"
#include "queuemanager.h"
#include "server.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSettings>

namespace MoleQueue {

Queue::Queue(const QString &queueName, QueueManager *parentManager) :
  QObject(parentManager), m_queueManager(parentManager),
  m_server((m_queueManager) ? m_queueManager->server() : NULL),
  m_name(queueName)
{
  qRegisterMetaType<Program*>("MoleQueue::Program*");
  qRegisterMetaType<const Program*>("const MoleQueue::Program*");
  qRegisterMetaType<IdType>("MoleQueue::IdType");
  qRegisterMetaType<JobState>("MoleQueue::JobState");

  if (m_server) {
    connect(m_server->jobManager(),
            SIGNAL(jobAboutToBeRemoved(const MoleQueue::Job&)),
            this, SLOT(jobAboutToBeRemoved(const MoleQueue::Job&)));
  }
}

Queue::~Queue()
{
  QList<Program*> programList = m_programs.values();
  m_programs.clear();
  qDeleteAll(programList);
}

bool Queue::readSettings(const QString &stateFilename)
{
  return readJsonSettingsFromFile(stateFilename, false, true);
}

bool Queue::writeSettings() const
{
  QString fileName = stateFileName();
  if (fileName.isEmpty()) {
    Logger::logError(tr("Cannot write settings for Queue %1: Cannot determine "
                        "config filename.").arg(name()));
    return false;
  }

  // Create directory if needed.
  QDir queueDir(QFileInfo(fileName).dir());
  if (!queueDir.exists()) {
    if (!queueDir.mkpath(queueDir.absolutePath())) {
      Logger::logError(tr("Cannot write settings for Queue %1: Cannot create "
                          "config directory %2.").arg(name())
                       .arg(queueDir.absolutePath()));
      return false;
    }
  }

  return writeJsonSettingsToFile(fileName, false, true);

}

bool Queue::exportSettings(const QString &fileName, bool includePrograms) const
{
  return writeJsonSettingsToFile(fileName, true, includePrograms);
}

bool Queue::importSettings(const QString &fileName, bool includePrograms)
{
  return readJsonSettingsFromFile(fileName, true, includePrograms);
}

QString Queue::queueTypeFromFile(const QString &mqqFile)
{
  QString result;
  if (!QFile::exists(mqqFile))
    return result;

  QFile stateFile(mqqFile);
  if (!stateFile.open(QFile::ReadOnly | QFile::Text))
    return result;
  QByteArray inputText = stateFile.readAll();
  stateFile.close();

  // Try to read existing data in
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(inputText.begin(), inputText.end(), root, false))
    return result;

  if (root.isObject() && root.isMember("type") && root["type"].isString())
    result = QString(root["type"].asCString());

  return result;
}

QString Queue::stateFileName() const
{
  QString workDir;
  if (m_server) {
    workDir = m_server->workingDirectoryBase();
  }
  else {
    QSettings settings;
    workDir = settings.value("workingDirectoryBase").toString();
  }

  if (workDir.isEmpty())
    return "";

  return QDir::cleanPath(workDir + "/config/queues/" + name() + ".mqq");
}

bool Queue::writeJsonSettingsToFile(const QString &stateFilename,
                                    bool exportOnly,
                                    bool includePrograms) const
{
  QFile stateFile(stateFilename);
  if (!stateFile.open(QFile::ReadWrite | QFile::Text | QFile::Truncate)) {
    Logger::logError(tr("Cannot save queue information for queue %1 in %2: "
                        "Cannot open file.").arg(name()).arg(stateFilename));
    return false;
  }

  Json::Value root(Json::objectValue);
  if (!this->writeJsonSettings(root, exportOnly, includePrograms)) {
    stateFile.close();
    return false;
  }

  if (!root.isObject()) {
    Logger::logError(tr("Internal error writing state for queue %1 in %2:"
                        " root is not an object!")
                     .arg(name()).arg(stateFilename));
    stateFile.close();
    return false;
  }

  // Write the data back out:
  std::string outputText = root.toStyledString();
  stateFile.write(QByteArray(outputText.c_str()));
  stateFile.close();

  return true;
}

bool Queue::readJsonSettingsFromFile(const QString &stateFilename,
                                     bool importOnly,
                                     bool includePrograms)
{
  if (!QFile::exists(stateFilename))
    return false;

  QFile stateFile(stateFilename);
  if (!stateFile.open(QFile::ReadOnly | QFile::Text)) {
    Logger::logError(tr("Cannot read queue information from %1.")
                     .arg(stateFilename));
    return false;
  }

  // Try to read existing data in
  Json::Value root;
  Json::Reader reader;
  QByteArray inputText = stateFile.readAll();
  if (!reader.parse(inputText.begin(), inputText.end(), root, false)) {
    Logger::logError(tr("Cannot parse queue state from %1:\n%2")
                     .arg(stateFilename).arg(inputText.data()));
    stateFile.close();
    return false;
  }

  if (!root.isObject()) {
    Logger::logError(tr("Error reading queue state from %1: "
                        "root is not an object!\n%2")
                     .arg(stateFilename)
                     .arg(inputText.data()));
    stateFile.close();
    return false;
  }

  return readJsonSettings(root, importOnly, includePrograms);
}

bool Queue::writeJsonSettings(Json::Value &root, bool exportOnly,
                              bool includePrograms) const
{
  root["type"] = typeName().toStdString();
  root["launchTemplate"] = m_launchTemplate.toStdString();
  root["launchScriptName"] = m_launchScriptName.toStdString();

  if (!exportOnly) {
    Json::Value jobIdMap(Json::objectValue);
    QList<IdType> keys = m_jobs.keys();
    for (int i = 0; i < keys.size(); ++i)
      jobIdMap[QString::number(keys[i]).toStdString()] = m_jobs[keys[i]];
    root["jobIdMap"] = jobIdMap;
  }

  if (includePrograms) {
    Json::Value programsObject(Json::objectValue);
    foreach (const Program *prog, programs()) {
      Json::Value programObject(Json::objectValue);
      if (prog->writeJsonSettings(programObject, exportOnly)) {
        programsObject[prog->name().toStdString()] = programObject;
      }
      else {
        Logger::logError(tr("Could not save program %1 in queue %2's settings.")
                         .arg(prog->name(), name()));
      }
    }
    root["programs"] = programsObject;
  }

  return true;
}

bool Queue::readJsonSettings(const Json::Value &root, bool importOnly,
                             bool includePrograms)
{
  // Verify JSON:
  if (!root.isObject() ||
      !root["type"].isString() ||
      !root["launchTemplate"].isString() ||
      !root["launchScriptName"].isString() ||
      (root.isMember("programs") && !root["programs"].isObject())) {
    Logger::logError(tr("Error reading queue settings: Invalid format:\n%1")
                     .arg(QString(root.toStyledString().c_str())));
    return false;
  }

  if (typeName() != QString(root["type"].asCString())) {
    Logger::logError(tr("Error reading queue settings: Types do not match.\n"
                        "Expected %1, got %2.").arg(typeName())
                     .arg(QString(root["type"].asCString())));
    return false;
  }

  QMap<IdType, IdType> jobIdMap;
  if (!importOnly && root.isMember("jobIdMap")) {
    const Json::Value &jobIdObject = root["jobIdMap"];

    if (!jobIdObject.isObject()) {
      Logger::logError(tr("Error reading queue settings: Invalid format:\n%1")
                       .arg(QString(root.toStyledString().c_str())));
      return false;
    }

    for (Json::ValueIterator it = jobIdObject.begin(),
         it_end = jobIdObject.end(); it != it_end; ++it) {
      QString jobIdStr(it.memberName());
      bool ok = false;
      IdType jobId = static_cast<IdType>(jobIdStr.toULongLong(&ok));
      if (!ok || !(*it).isIntegral()) {
        Logger::logError(tr("Error reading queue settings: Invalid format:\n%1")
                         .arg(QString(root.toStyledString().c_str())));
        return false;
      }
      IdType moleQueueId = static_cast<IdType>((*it).asLargestUInt());
      jobIdMap.insert(jobId, moleQueueId);
    }
  }

  QMap<QString, Program*> programMap;
  if (includePrograms && root.isMember("programs")) {
    const Json::Value &programObject = root["programs"];

    if (!programObject.isObject()) {
      Logger::logError(tr("Error reading queue settings: Invalid format:\n%1")
                       .arg(QString(root.toStyledString().c_str())));
      return false;
    }

    for (Json::ValueIterator it = programObject.begin(),
         it_end = programObject.end(); it != it_end; ++it) {

      QString progName(it.memberName());
      if (progName.isEmpty()) {
        Logger::logError(tr("Error: empty program name in queue %1 config.")
                         .arg(name()));
        return false;
      }

      Program *prog = new Program(this);
      programMap.insert(progName, prog);
      prog->setName(progName);
      if (!prog->readJsonSettings(*it, importOnly)) {
        Logger::logError(tr("Error loading configuration for program %1 in "
                            "queue %2.").arg(progName).arg(name()));
        qDeleteAll(programMap.values());
        return false;
      }
    }
  }

  // Everything is verified -- go ahead and update queue.
  m_launchTemplate = QString(root["launchTemplate"].asCString());
  m_launchScriptName = QString(root["launchScriptName"].asCString());

  if (!importOnly)
    m_jobs = jobIdMap;

  if (includePrograms) {
    for (QMap<QString, Program*>::const_iterator it = programMap.constBegin(),
         it_end = programMap.constEnd(); it != it_end; ++it) {
      if (!addProgram(it.value())) {
        Logger::logDebugMessage(tr("Cannot add program '%1' to queue '%2': "
                                   "program name already exists!")
                                .arg(it.key()).arg(name()));
        it.value()->deleteLater();
        continue;
      }
    }
  }

  return true;
}

AbstractQueueSettingsWidget* Queue::settingsWidget()
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

  if (newProgram->parent() != this)
    newProgram->setParent(this);

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

void Queue::replaceLaunchScriptKeywords(QString &launchScript, const Job &job,
                                        bool addNewline)
{
  launchScript.replace("$$moleQueueId$$", QString::number(job.moleQueueId()));

  launchScript.replace("$$numberOfCores$$",
                       QString::number(job.numberOfCores()));

  job.replaceLaunchScriptKeywords(launchScript);

  // Remove any unreplaced keywords
  QRegExp expr("[^\\$]?(\\${2,3}[^\\$\\s]+\\${2,3})[^\\$]?");
  while (expr.indexIn(launchScript) != -1) {
    Logger::logWarning(tr("Unhandled keyword in launch script: %1. Removing.")
                       .arg(expr.cap(1)), job.moleQueueId());
    launchScript.remove(expr.cap(1));
  }

  // Add newline at end if not present
  if (addNewline && !launchScript.isEmpty() &&
      !launchScript.endsWith(QChar('\n'))) {
    launchScript.append(QChar('\n'));
  }
}

bool Queue::writeInputFiles(const Job &job)
{
  QString workdir = job.localWorkingDirectory();

  // Lookup program.
  if (!m_server) {
    Logger::logError(tr("Queue '%1' cannot locate Server instance!")
                     .arg(m_name),
                     job.moleQueueId());
    return false;
  }
  const Program *program = lookupProgram(job.program());
  if (!program) {
    Logger::logError(tr("Queue '%1' cannot locate program '%2'!")
                     .arg(m_name).arg(job.program()),
                     job.moleQueueId());
    return false;
  }

  // Create directory
  QDir dir (workdir);

  /// Send a warning but don't bail if the path already exists.
  if (dir.exists()) {
    Logger::logWarning(tr("Directory already exists: %1")
                       .arg(dir.absolutePath()),
                       job.moleQueueId());
  }
  else {
    if (!dir.mkpath(dir.absolutePath())) {
      Logger::logError(tr("Cannot create directory: %1")
                       .arg(dir.absolutePath()),
                       job.moleQueueId());
      return false;
    }
  }

  // Create input files
  FileSpecification inputFile = job.inputFile();
  if (!program->inputFilename().isEmpty() && inputFile.isValid()) {
    /// @todo Allow custom file names, only specify extension in program.
    /// Use $$basename$$ keyword replacement.
    inputFile.writeFile(dir, program->inputFilename());
  }

  // Write additional input files
  QList<FileSpecification> additionalInputFiles = job.additionalInputFiles();
  foreach (const FileSpecification &filespec, additionalInputFiles) {
    if (!filespec.isValid()) {
      Logger::logError(tr("Writing additional input files...invalid FileSpec:\n"
                          "%1").arg(filespec.asJsonString()),
                       job.moleQueueId());
      return false;
    }
    QFileInfo target(dir.absoluteFilePath(filespec.filename()));
    switch (filespec.format()) {
    default:
    case FileSpecification::InvalidFileSpecification:
      Logger::logWarning(tr("Cannot write input file. Invalid filespec:\n%1")
                         .arg(filespec.asJsonString()), job.moleQueueId());
      continue;
    case FileSpecification::PathFileSpecification: {
      QFileInfo source(filespec.filepath());
      if (!source.exists()) {
        Logger::logError(tr("Writing additional input files...Source file "
                            "does not exist! %1")
                         .arg(source.absoluteFilePath()), job.moleQueueId());
        return false;
      }
      if (source == target) {
        Logger::logWarning(tr("Refusing to copy additional input file...source "
                              "and target refer to the same file!\nSource: %1"
                              "\nTarget: %2").arg(source.absoluteFilePath())
                           .arg(target.absoluteFilePath()), job.moleQueueId());
        continue;
      }
    }
    case FileSpecification::ContentsFileSpecification:
      if (target.exists()) {
        Logger::logWarning(tr("Writing additional input files...Overwriting "
                              "existing file: '%1'")
                           .arg(target.absoluteFilePath()), job.moleQueueId());
        QFile::remove(target.absoluteFilePath());
      }
      filespec.writeFile(dir);
      continue;
    }
  }

  // Do we need a driver script?
  const QueueLocal *localQueue = qobject_cast<const QueueLocal*>(this);
  const QueueRemote *remoteQueue = qobject_cast<const QueueRemote*>(this);
  if ((localQueue && program->launchSyntax() == Program::CUSTOM) ||
      remoteQueue) {
    QFile launcherFile (dir.absoluteFilePath(launchScriptName()));
    if (!launcherFile.open(QFile::WriteOnly | QFile::Text)) {
      Logger::logError(tr("Cannot open file for writing: %1.")
                       .arg(launcherFile.fileName()),
                       job.moleQueueId());
      return false;
    }
    QString launchString = program->launchTemplate();

    replaceLaunchScriptKeywords(launchString, job);

    launcherFile.write(launchString.toLatin1());
    if (!launcherFile.setPermissions(
          launcherFile.permissions() | QFile::ExeUser)) {
      Logger::logError(tr("Cannot set executable permissions on file: %1.")
                       .arg(launcherFile.fileName()),
                       job.moleQueueId());
      return false;
    }
    launcherFile.close();
  }

  return true;
}

bool Queue::addJobFailure(IdType moleQueueId)
{
  if (!m_failureTracker.contains(moleQueueId)) {
    m_failureTracker.insert(moleQueueId, 1);
    return true;
  }

  int failures = ++m_failureTracker[moleQueueId];

  if (failures > 3) {
    Logger::logError(tr("Maximum number of retries for job %1 exceeded.")
                     .arg(moleQueueId), moleQueueId);
    clearJobFailures(moleQueueId);
    return false;
  }

  return true;
}

void Queue::jobAboutToBeRemoved(const Job &job)
{
  m_failureTracker.remove(job.moleQueueId());
  m_jobs.remove(job.queueId());
}

void Queue::cleanLocalDirectory(const Job &job)
{
  if (!FileSystemTools::recursiveRemoveDirectory(job.localWorkingDirectory(),
                                                 true)) {
    Logger::logError(tr("Cannot remove '%1' from local filesystem.")
                     .arg(job.localWorkingDirectory()));
  }
}

} // End namespace
