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
    m_executableFilePath(other.m_executableFilePath),
    m_executableName(other.m_executableName),
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
  m_executableFilePath = other.m_executableFilePath;
  m_executableName = other.executableName();
  return *this;
}

void OpenWithActionFactory::readSettings(QSettings &settings)
{
  m_executableFilePath = settings.value("executableFilePath").toString();
  m_executableName = settings.value("executableName").toString();
  JobActionFactory::readSettings(settings);
}

void OpenWithActionFactory::writeSettings(QSettings &settings) const
{
  settings.setValue("executableFilePath", m_executableFilePath);
  settings.setValue("executableName", m_executableName);
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
    QStringList filenames = m_filenames.keys();
    m_menuText = tr("Open '%1' in %2").arg(job.description(), m_executableName);
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
  if (!action)
    return;

  // The sender was a QAction. Is its data a jobs?
  Job job = action->data().value<Job>();
  if (!job.isValid())
    return;

  // Filename was set?
  QString filename = action->property("filename").toString();
  if (!QFileInfo(filename).exists())
    return;

  QSettings settings;
  settings.beginGroup("ActionFactory/OpenWith/" + m_executableName);
  m_executableFilePath = settings.value("path", m_executableFilePath).toString();

  // Ensure that the path to the executable is valid
  if (m_executableFilePath.isEmpty() || !QFile::exists(m_executableFilePath) ||
      !(QFile::permissions(m_executableFilePath) & QFile::ExeUser)) {
    // Invalid path -- search system path:
    if (!searchPathForExecutable(m_executableName)) {
      // not found in path. Ask user.
      m_executableFilePath = QFileDialog::getOpenFileName(
            NULL, tr("Specify location of %1").arg(m_executableName),
            m_executableFilePath, m_executableName, 0);
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

  // Should be ready to go!
  QProcess::startDetached(QString("\"%1\" \"%2\"").arg(m_executableFilePath,
                                                       filename));
}

bool OpenWithActionFactory::searchPathForExecutable(const QString &exec)
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if (!env.contains("PATH"))
    return false;

  static QRegExp pathSplitter = QRegExp(
#ifdef _WIN32
        ";"
#else // WIN32
        ":"
#endif// WIN32
        );
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
