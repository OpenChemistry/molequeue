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

#include "openwithactionfactory.h"

#include "../job.h"
#include "../program.h"
#include "../queue.h"
#include "../queuemanager.h"
#include "../server.h"

#include <QtGui/QAction>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QVariant>

Q_DECLARE_METATYPE(QList<const MoleQueue::Job*>)

namespace MoleQueue
{

OpenWithActionFactory::OpenWithActionFactory()
  : JobActionFactory()
{
  qRegisterMetaType<QList<const Job*> >("QList<const Job*>");
  m_isMultiJob = true;
  m_flags |= JobActionFactory::ContextItem;
}

OpenWithActionFactory::OpenWithActionFactory(const OpenWithActionFactory &other)
  : JobActionFactory(other),
    m_executableFilePath(other.m_executableFilePath),
    m_executableName(other.m_executableName)
{
}

OpenWithActionFactory::~OpenWithActionFactory()
{
}

OpenWithActionFactory &OpenWithActionFactory::operator =(
    const OpenWithActionFactory &other)
{
  this->JobActionFactory::operator=(other);
  m_executableFilePath = other.m_executableFilePath;
  m_executableName = other.executableName();
  return *this;
}

void OpenWithActionFactory::readSettings(QSettings &settings)
{
  m_executableFilePath = settings.value("executableFilePath").toString();
  m_executableName = settings.value("executableName").toString();
  this->JobActionFactory::readSettings(settings);
}

void OpenWithActionFactory::writeSettings(QSettings &settings) const
{
  settings.setValue("executableFilePath", m_executableFilePath);
  settings.setValue("executableName", m_executableName);
  this->JobActionFactory::writeSettings(settings);
}

void OpenWithActionFactory::clearJobs()
{
  this->JobActionFactory::clearJobs();
}

bool OpenWithActionFactory::isValidForJob(const Job *) const
{
  return false;
}

bool OpenWithActionFactory::hasValidActions() const
{
  return this->JobActionFactory::hasValidActions();
}

QList<QAction *> OpenWithActionFactory::createActions()
{
  QList<QAction*> result;

  if (m_attemptedJobAdditions == 1) {
    QAction *newAction = new QAction (
          tr("Open '%1' in %2...").arg(m_jobs.first()->description())
          .arg(m_executableName), NULL);
    newAction->setData(QVariant::fromValue(m_jobs));
    connect(newAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
    result << newAction;
  }
  else if (m_attemptedJobAdditions > 1) {
    QAction *newAction = new QAction (NULL);
    if (static_cast<unsigned int>(m_jobs.size()) == m_attemptedJobAdditions)
      newAction->setText(tr("Open %1 jobs in %2").arg(m_jobs.size())
                         .arg(m_executableName));
    else
      newAction->setText(tr("Open %1 of %2 selected jobs in %3...")
                         .arg(m_jobs.size()).arg(m_attemptedJobAdditions)
                         .arg(m_executableName));
    newAction->setData(QVariant::fromValue(m_jobs));
    connect(newAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
    result << newAction;
  }

  return result;
}

unsigned int OpenWithActionFactory::usefulness() const
{
  return 800;
}

void OpenWithActionFactory::actionTriggered()
{
  QAction *action = qobject_cast<QAction*>(this->sender());
  if (!action)
    return;

  // The sender was a QAction. Is its data a list of jobs?
  QList<const Job *> jobs = action->data().value<QList<const Job*> >();
  if (!jobs.size())
    return;

  QSettings settings;
  settings.beginGroup("ActionFactory/OpenWith/" + m_executableName);
  m_executableFilePath = settings.value("path", m_executableFilePath).toString();

  // Ensure that the path to the executable is valid
  if (m_executableFilePath.isEmpty() || !QFile::exists(m_executableFilePath) ||
      !(QFile::permissions(m_executableFilePath) & QFile::ExeUser)) {
    // Invalid path -- search system path:
    if (!this->searchPathForExecutable(m_executableName)) {
      // not found in path. Ask user.
      m_executableFilePath =
          QFileDialog::getOpenFileName(NULL, tr("Specify location of %1")
                                       .arg(m_executableName), m_executableFilePath,
                                       m_executableName, 0);
      // Check for user cancel:
      if (m_executableFilePath.isNull())
        return;
    }

    // Does the new path exist?
    if (!QFile::exists(m_executableFilePath)) {
      QMessageBox::critical(NULL, tr("Executable does not exist!"),
                            tr("The executable file at %1 does not exist!")
                            .arg(m_executableFilePath));
      return;
    }

    // Is the target executable?
    if (!(QFile::permissions(m_executableFilePath) & QFile::ExeUser)) {
      QMessageBox::critical(NULL, tr("File is not executable!"),
                            tr("The file at %1 is not executable and cannot "
                               "be used to open job output.")
                            .arg(m_executableFilePath));
      return;
    }
  }

  settings.setValue("path", m_executableFilePath);
  settings.endGroup();

  // Attempt to lookup program for output filenames
  QueueManager *queueManager = m_server ? m_server->queueManager() : NULL;

  foreach (const Job *job, jobs) {
    QString outputFile = job->outputDirectory();

    Queue *queue = queueManager ? queueManager->lookupQueue(job->queue())
                                  : NULL;
    Program *program = queue ? queue->lookupProgram(job->program()) : NULL;

    if (program)
      outputFile = QUrl::fromLocalFile(
            outputFile + "/" + program->outputFilename()).toLocalFile();

    // Output file does not exist -- prompt user for which file to open
    if (!QFile::exists(outputFile)) {
      outputFile =
          QFileDialog::getOpenFileName(NULL, tr("Select output file to open in "
                                                "%1").arg(m_executableName),
                                       outputFile);
      // User cancel
      if (outputFile.isNull())
        return;
    }

    // Should be ready to go!
    QProcess::startDetached(m_executableFilePath + " " + outputFile);
  }
}

bool OpenWithActionFactory::searchPathForExecutable(const QString &exec)
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if (!env.contains("PATH"))
    return false;

  static QRegExp pathSplitter = QRegExp(":");
  QStringList paths =
      env.value("PATH").split(pathSplitter, QString::SkipEmptyParts);

  foreach (const QString &path, paths) {
    QString testPath = QUrl::fromLocalFile(path + "/" + exec).toLocalFile();
    if (!QFile::exists(testPath))
      continue;
    m_executableFilePath = testPath;
    return true;
  }

  return false;
}

} // end namespace MoleQueue
