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

#include "viewjoblogactionfactory.h"

#include "logwindow.h"
#include "job.h"

#include <QtGui/QAction>

Q_DECLARE_METATYPE(QList<MoleQueue::Job>)

namespace MoleQueue {

ViewJobLogActionFactory::ViewJobLogActionFactory() :
  MoleQueue::JobActionFactory(),
m_logWindowParent(NULL)
{
  qRegisterMetaType<QList<Job> >("QList<Job>");
  m_isMultiJob = false;
  m_flags |= JobActionFactory::ContextItem;
}

ViewJobLogActionFactory::~ViewJobLogActionFactory()
{
  qDeleteAll(m_windowMap.values());
  m_windowMap.clear();
}

bool ViewJobLogActionFactory::isValidForJob(const Job &job) const
{
  return job.isValid();
}

QList<QAction *> ViewJobLogActionFactory::createActions()
{
  QList<QAction*> result;

  QAction *newAction = NULL;

  if (m_attemptedJobAdditions == 1 && m_jobs.size() == 1) {
    newAction = new QAction (tr("View log for job '%1'...")
                             .arg(m_jobs.first().description()), NULL);
  }

  if (newAction) {
    newAction->setData(QVariant::fromValue(m_jobs));
    connect(newAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
    result << newAction;
  }

  return result;
}

void ViewJobLogActionFactory::setLogWindowParent(QWidget *widgy)
{
  m_logWindowParent = widgy;
}

void ViewJobLogActionFactory::actionTriggered()
{
  QAction *action = qobject_cast<QAction*>(sender());
  if (!action)
    return;

  // The sender was a QAction. Is its data a list of jobs?
  QList<Job> jobs = action->data().value<QList<Job> >();
  if (jobs.size() != 1 || !jobs.first().isValid())
    return;


  IdType moleQueueId = jobs.first().moleQueueId();
  LogWindow *logWindow = m_windowMap.value(moleQueueId, NULL);
  if (!logWindow) {
    logWindow = new LogWindow(m_logWindowParent, moleQueueId);
    m_windowMap.insert(moleQueueId, logWindow);
    connect(logWindow, SIGNAL(aboutToClose()),
            this, SLOT(removeSenderFromMap()));
  }

  logWindow->show();
  logWindow->raise();
}

void ViewJobLogActionFactory::removeSenderFromMap()
{
  LogWindow *deadWindow = qobject_cast<LogWindow*>(this->sender());
  if (!deadWindow)
    return;

  IdType moleQueueId = m_windowMap.key(deadWindow, InvalidId);
  if (moleQueueId != InvalidId)
    m_windowMap.remove(moleQueueId);
  deadWindow->deleteLater();
}

} // namespace MoleQueue
