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

#include "opendirectoryactionfactory.h"

#include "../job.h"

#include <QtGui/QAction>
#include <QtGui/QDesktopServices>

#include <QtCore/QUrl>

Q_DECLARE_METATYPE(QList<MoleQueue::Job>)

namespace MoleQueue
{

OpenDirectoryActionFactory::OpenDirectoryActionFactory() :
  JobActionFactory()
{
  qRegisterMetaType<QList<Job> >("QList<Job>");
  qRegisterMetaType<QList<Job> >("QList<MoleQueue::Job>");
  m_isMultiJob = true;
  m_flags |= JobActionFactory::ContextItem;
}

OpenDirectoryActionFactory::~OpenDirectoryActionFactory()
{
}

bool OpenDirectoryActionFactory::isValidForJob(const Job &job) const
{
  return job.isValid() && !job.outputDirectory().isEmpty();
}

QList<QAction *> OpenDirectoryActionFactory::createActions()
{
  QList<QAction *> result;
  if (m_attemptedJobAdditions == 1) {
    QAction *newAction = new QAction (
          tr("Open '%1' in file browser...").arg(m_jobs.first().description()),
          NULL);
    newAction->setData(QVariant::fromValue(m_jobs));
    connect(newAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
    result << newAction;
  }
  else if (m_attemptedJobAdditions > 1) {
    QAction *newAction = new QAction (NULL);
    if (static_cast<unsigned int>(m_jobs.size()) == m_attemptedJobAdditions) {
      newAction->setText(tr("Open %1 jobs in file browser")
                         .arg(m_jobs.size()));
    }
    else {
      newAction->setText(tr("Open %1 of %2 selected jobs in file browser...")
                         .arg(m_jobs.size()).arg(m_attemptedJobAdditions));
    }
    newAction->setData(QVariant::fromValue(m_jobs));
    connect(newAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
    result << newAction;
  }
  return result;
}

void OpenDirectoryActionFactory::actionTriggered()
{
  QAction *action = qobject_cast<QAction*>(sender());
  if (!action)
    return;

  // The sender was a QAction. Is its data a list of jobs?
  QList<Job> jobs = action->data().value<QList<Job> >();
  if (!jobs.size())
    return;

  foreach (const Job &job, jobs) {
    if (job.isValid())
      QDesktopServices::openUrl(QUrl::fromLocalFile(job.outputDirectory()));
  }
}

} // end namespace MoleQueue
