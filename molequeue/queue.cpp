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
  QObject(parentManager), m_queueManager(parentManager),
  m_server((m_queueManager) ? m_queueManager->server() : NULL),
  m_name(queueName)
{
  qRegisterMetaType<Program*>("MoleQueue::Program*");
  qRegisterMetaType<const Program*>("const MoleQueue::Program*");
  qRegisterMetaType<IdType>("MoleQueue::IdType");
  qRegisterMetaType<JobState>("MoleQueue::JobState");

  if (m_server) {
    connect(this, SIGNAL(jobStateUpdate(MoleQueue::IdType,MoleQueue::JobState)),
            m_server->jobManager(), SLOT(updateJobState(MoleQueue::IdType,
                                                        MoleQueue::JobState)));
    connect(this, SIGNAL(queueIdUpdate(MoleQueue::IdType,MoleQueue::IdType)),
            m_server->jobManager(), SLOT(updateQueueId(MoleQueue::IdType,
                                                       MoleQueue::IdType)));
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

  QList<IdType> keys = m_jobs.keys();
  settings.beginWriteArray("JobIdMap", keys.size());
  for (int i = 0; i < keys.size(); ++i) {
    settings.setArrayIndex(i);
    settings.setValue("queueId", keys[i]);
    settings.setValue("moleQueueId", m_jobs[keys[i]]);
  }
  settings.endArray(); // JobIdMap

  settings.setValue("programs", this->programNames());
  settings.beginGroup("Programs");
  foreach (const Program *prog, this->programs()) {
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

bool Queue::writeInputFiles(const Job *job)
{
  /// @todo Emit job error signals instead of qWarnings

  QString workdir = job->localWorkingDirectory();

  // Lookup program.
  if (!m_server) {
    qWarning() << Q_FUNC_INFO << "Error: Cannot locate server.";
    return false;
  }
  const Program *program = this->lookupProgram(job->program());
  if (!program) {
    qWarning() << Q_FUNC_INFO << "Error: Unknown program:" << job->program();
    return false;
  }

  // Create directory
  QDir dir (workdir);

  /// @todo Should this be a failure?
  if (dir.exists()) {
    qWarning() << Q_FUNC_INFO << "Error: Directory already exists:"
               << dir.absolutePath();
    return false;
  }
  if (!dir.mkpath(dir.absolutePath())) {
    qWarning() << Q_FUNC_INFO << "Error: Cannot create directory:"
               << dir.absolutePath();
    return false;
  }

  // Create input files
  QString inputFilePath = dir.absoluteFilePath(program->inputFilename());
  if (job->inputAsPath().isEmpty()) {
    // Open a file and write the input string to it.
    QFile inputFile (inputFilePath);
    if (!inputFile.open(QFile::WriteOnly | QFile::Text)) {
      qWarning() << Q_FUNC_INFO << "Error: Cannot open file for writing:"
                 << inputFilePath;
      return false;
    }
    inputFile.write(job->inputAsString().toLatin1());
    inputFile.close();
  }
  else {
    // Copy the input file to the input path
    if (!QFile::copy(job->inputAsPath(), inputFilePath)) {
      qWarning() << Q_FUNC_INFO << "Error: Cannot copy file"
                 << job->inputAsPath() << "to" << inputFilePath;
      return false;
    }
  }

  // Do we need a driver script?
  const QueueLocal *localQueue =
      qobject_cast<const QueueLocal*>(program->queue());
  const QueueRemote *remoteQueue =
      qobject_cast<const QueueRemote*>(program->queue());
  if ((localQueue && program->launchSyntax() == Program::CUSTOM) ||
      remoteQueue) {
    QFile launcherFile (dir.absoluteFilePath(this->launchScriptName()));
    if (!launcherFile.open(QFile::WriteOnly | QFile::Text)) {
      qWarning() << Q_FUNC_INFO << "Error: Cannot open file for writing:"
                 << launcherFile.fileName();
      return false;
    }
    QString launchString = program->launchTemplate();
    if (launchString.contains("$$moleQueueId$$")) {
      launchString.replace("$$moleQueueId$$",
                           QString::number(job->moleQueueId()));
    }
    launcherFile.write(launchString.toLatin1());
    if (!launcherFile.setPermissions(
          launcherFile.permissions() | QFile::ExeUser)) {
      qWarning() << Q_FUNC_INFO << "Error: Cannot set permissions on file:"
                 << launcherFile.fileName();
      return false;
    }
    launcherFile.close();
  }

  return true;
}

} // End namespace
