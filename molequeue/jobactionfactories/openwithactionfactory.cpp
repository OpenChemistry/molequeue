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
#include "../logger.h"
#include "../program.h"
#include "../queue.h"
#include "../queuemanager.h"
#include "../server.h"

#include <QtGui/QAction>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

namespace MoleQueue
{

OpenWithActionFactory::OpenWithActionFactory()
  : JobActionFactory()
{
  qRegisterMetaType<Job>("Job");
  m_isMultiJob = false;
  m_flags |= JobActionFactory::ContextItem;
}

OpenWithActionFactory::OpenWithActionFactory(const OpenWithActionFactory &other)
  : JobActionFactory(other),
    m_executable(other.m_executable),
    m_menuText(other.m_menuText)
{
}

OpenWithActionFactory::~OpenWithActionFactory()
{
}

OpenWithActionFactory &OpenWithActionFactory::operator =(
    const OpenWithActionFactory &other)
{
  JobActionFactory::operator=(other);
  m_executable = other.executable();
  return *this;
}

void OpenWithActionFactory::readSettings(QSettings &settings)
{
  m_executable = settings.value("executable").toString();
  JobActionFactory::readSettings(settings);
}

void OpenWithActionFactory::writeSettings(QSettings &settings) const
{
  settings.setValue("executable", m_executable);
  JobActionFactory::writeSettings(settings);
}

void OpenWithActionFactory::clearJobs()
{
  JobActionFactory::clearJobs();
  m_filenames.clear();
  m_menuText = QString();
}

bool OpenWithActionFactory::useMenu() const
{
  return true;
}

QString OpenWithActionFactory::menuText() const
{
  return m_menuText;
}

QList<QAction *> OpenWithActionFactory::createActions()
{
  QList<QAction*> result;

  if (m_attemptedJobAdditions == 1 && m_jobs.size() == 1) {
    const Job &job = m_jobs.first();
    m_menuText = tr("Open '%1' with %2")
        .arg(job.description(), name());
    QStringList filenames = m_filenames.keys();
    foreach (const QString &filename, filenames) {
      QAction *newAction = new QAction(filename, NULL);
      newAction->setData(QVariant::fromValue(job));
      newAction->setProperty("filename", m_filenames.value(filename));
      connect(newAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
      result << newAction;
    }
  }

  return result;
}

unsigned int OpenWithActionFactory::usefulness() const
{
  return 800;
}

void OpenWithActionFactory::actionTriggered()
{
  QAction *action = qobject_cast<QAction*>(sender());
  if (!action) {
    Logger::logWarning(tr("OpenWithActionFactory::actionTriggered: Sender is "
                          "not a QAction!"));
    return;
  }

  // The sender was a QAction. Is its data a job?
  Job job = action->data().value<Job>();
  if (!job.isValid()) {
    Logger::logWarning(tr("OpenWithActionFactory::actionTriggered: Action data "
                          "is not a Job."));
    return;
  }

  // Filename was set?
  QString filename = action->property("filename").toString();
  if (!QFileInfo(filename).exists()) {
    Logger::logWarning(tr("OpenWithActionFactory::actionTriggered: No filename "
                          "associated with job."), job.moleQueueId());
    return;
  }

  // Should be ready to go!
  qint64 pid = -1;
  QString workDir = QFileInfo(filename).absolutePath();
  bool ok = QProcess::startDetached(
        QString("%1").arg(m_executable),
        QStringList() << filename, workDir, &pid);

  // pid may be set to zero in certain cases in the UNIX QProcess implementation
  if (ok && pid >= 0) {
    Logger::logDebugMessage(tr("Running '%1 %2' in '%3' (PID=%4)",
                               "1 is an executable, 2 is a filename, 3 is a "
                               "directory, and 4 is a process id.")
                            .arg(m_executable).arg(filename).arg(workDir)
                            .arg(QString::number(pid)), job.moleQueueId());
  }
  else {
    QString err = tr("Error while starting '%1 %2' in '%3'",
                     "1 is an executable, 2 is a filename, 3 is a directory.")
        .arg(m_executable).arg(filename).arg(workDir);
    Logger::logWarning(err, job.moleQueueId());
    QMessageBox::critical(NULL, tr("Cannot start process"), err);
  }
}

} // end namespace MoleQueue
