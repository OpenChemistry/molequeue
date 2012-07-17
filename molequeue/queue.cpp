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

#include "error.h"
#include "job.h"
#include "jobmanager.h"
#include "program.h"
#include "queues/local.h"
#include "queues/remote.h"
#include "queuemanager.h"
#include "server.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QSettings>

namespace MoleQueue {

Queue::Queue(const QString &queueName, QueueManager *parentManager) :
  Object(parentManager), m_queueManager(parentManager),
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

void Queue::readSettings(QSettings &settings)
{
  m_launchTemplate = settings.value("launchTemplate").toString();
  m_launchScriptName = settings.value("launchScriptName").toString();

  int jobIdMapSize = settings.beginReadArray("JobIdMap");
  for (int i = 0; i < jobIdMapSize; ++i) {
    settings.setArrayIndex(i);
    m_jobs.insert(static_cast<IdType>(settings.value("queueId").toInt()),
                  static_cast<IdType>(settings.value("moleQueueId").toInt()));
  }
  settings.endArray(); // JobIdMap

  QStringList progNames = settings.value("programs").toStringList();

  settings.beginGroup("Programs");
  foreach (const QString &progName, progNames) {
    settings.beginGroup(progName);

    Program *program = new Program (this);
    program->setName(progName);
    program->readSettings(settings);

    if (!addProgram(program)) {
      qWarning() << Q_FUNC_INFO << "Could not add program" << progName
                 << "to queue" << name() << "-- duplicate program name.";
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

  QList<IdType> keys = m_jobs.keys();
  settings.beginWriteArray("JobIdMap", keys.size());
  for (int i = 0; i < keys.size(); ++i) {
    settings.setArrayIndex(i);
    settings.setValue("queueId", keys[i]);
    settings.setValue("moleQueueId", m_jobs[keys[i]]);
  }
  settings.endArray(); // JobIdMap

  settings.setValue("programs", programNames());
  settings.beginGroup("Programs");
  foreach (const Program *prog, programs()) {
    settings.beginGroup(prog->name());
    prog->writeSettings(settings);
    settings.endGroup(); // prog->name()
  }
  settings.endGroup(); // "Programs"
}

QWidget* Queue::settingsWidget()
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

bool Queue::writeInputFiles(const Job &job)
{
  QString workdir = job.localWorkingDirectory();

  // Lookup program.
  if (!m_server) {
    Error err (tr("Queue '%1' cannot locate Server instance!").arg(m_name),
               Error::QueueError, this, job.moleQueueId());
    emit errorOccurred(err);
    return false;
  }
  const Program *program = lookupProgram(job.program());
  if (!program) {
    Error err (tr("Queue '%1' cannot locate program '%2'!")
               .arg(m_name).arg(job.program()),
               Error::QueueError, this, job.moleQueueId());
    emit errorOccurred(err);
    return false;
  }

  // Create directory
  QDir dir (workdir);

  /// Send a warning but don't bail if the path already exists.
  if (dir.exists()) {
    Error err (tr("Warning: Directory already exists: %1")
               .arg(dir.absolutePath()),
               Error::FileSystemError, this, job.moleQueueId());
    emit errorOccurred(err);
  }
  else {
    if (!dir.mkpath(dir.absolutePath())) {
      Error err (tr("Cannot create directory: %1")
                 .arg(dir.absolutePath()),
                 Error::FileSystemError, this, job.moleQueueId());
      emit errorOccurred(err);
      return false;
    }
  }

  // Create input files
  if (!program->inputFilename().isEmpty()) {
    QString inputFilePath = dir.absoluteFilePath(program->inputFilename());
    if (job.inputAsPath().isEmpty()) {
      // Open a file and write the input string to it.
      QFile inputFile (inputFilePath);
      if (!inputFile.open(QFile::WriteOnly | QFile::Text)) {
        Error err (tr("Cannot open file for writing: %1")
                   .arg(inputFilePath),
                   Error::FileSystemError, this, job.moleQueueId());
        emit errorOccurred(err);
        return false;
      }
      inputFile.write(job.inputAsString().toLatin1());
      inputFile.close();
    }
    else {
      // Copy the input file to the input path
      if (!QFile::copy(job.inputAsPath(), inputFilePath)) {
        Error err (tr("Cannot copy file '%1' --> '%2'.")
                   .arg(job.inputAsPath()).arg(inputFilePath),
                   Error::FileSystemError, this, job.moleQueueId());
        emit errorOccurred(err);
        return false;
      }
    }
  }

  // Do we need a driver script?
  /// @todo Find a better way to check if a driver script is needed.
  const QueueLocal *localQueue =
      qobject_cast<const QueueLocal*>(this);
  const QueueRemote *remoteQueue =
      qobject_cast<const QueueRemote*>(this);
  if ((localQueue && program->launchSyntax() == Program::CUSTOM) ||
      remoteQueue) {
    QFile launcherFile (dir.absoluteFilePath(launchScriptName()));
    if (!launcherFile.open(QFile::WriteOnly | QFile::Text)) {
      Error err (tr("Cannot open file for writing: %1.")
                 .arg(launcherFile.fileName()),
                 Error::FileSystemError, this, job.moleQueueId());
      emit errorOccurred(err);
      return false;
    }
    QString launchString = program->launchTemplate();
    if (launchString.contains("$$moleQueueId$$")) {
      launchString.replace("$$moleQueueId$$",
                           QString::number(job.moleQueueId()));
    }
    launcherFile.write(launchString.toLatin1());
    if (!launcherFile.setPermissions(
          launcherFile.permissions() | QFile::ExeUser)) {
      Error err (tr("Cannot set executable permissions on file: %1.")
                 .arg(launcherFile.fileName()),
                 Error::FileSystemError, this, job.moleQueueId());
      emit errorOccurred(err);
      return false;
    }
    launcherFile.close();
  }

  return true;
}

void Queue::jobAboutToBeRemoved(const Job &job)
{
  m_jobs.remove(job.queueId());
}

} // End namespace
