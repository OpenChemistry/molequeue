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

#include "killjobactionfactory.h"

#include "../job.h"
#include "../queue.h"
#include "../queuemanager.h"
#include "../server.h"
#include "../molequeueglobal.h"

#include <QtGui/QAction>
#include <QtGui/QMessageBox>

Q_DECLARE_METATYPE(QList<MoleQueue::Job>)

namespace MoleQueue
{

KillJobActionFactory::KillJobActionFactory() :
  JobActionFactory()
{
  qRegisterMetaType<QList<Job> >("QList<Job>");
  m_isMultiJob = true;
  m_flags |= JobActionFactory::ContextItem;
}

KillJobActionFactory::~KillJobActionFactory()
{
}

bool KillJobActionFactory::isValidForJob(const Job &job) const
{
  switch (job.jobState()) {
  case MoleQueue::Accepted:
  case MoleQueue::LocalQueued:
  case MoleQueue::Submitted:
  case MoleQueue::RemoteQueued:
  case MoleQueue::RunningLocal:
  case MoleQueue::RunningRemote:
  case MoleQueue::ErrorState:
    return true;
  case MoleQueue::Unknown:
  case MoleQueue::None:
  case MoleQueue::Finished:
  case MoleQueue::Killed:
  default:
    break;
  }
  return false;
}

QList<QAction *> KillJobActionFactory::createActions()
{
  QList<QAction*> result;

  QAction *newAction = NULL;

  if (m_attemptedJobAdditions == 1 && m_jobs.size() == 1) {
    newAction = new QAction (tr("Cancel job '%1'...")
                             .arg(m_jobs.first().description()), NULL);
  }
  else if (m_attemptedJobAdditions > 1) {
    newAction = new QAction (NULL);
    if (static_cast<unsigned int>(m_jobs.size()) == m_attemptedJobAdditions) {
      newAction->setText(tr("Cancel %1 jobs...").arg(m_jobs.size()));
    }
    else {
      newAction->setText(tr("Cancel %1 of %2 selected jobs...")
                         .arg(m_jobs.size()).arg(m_attemptedJobAdditions));
    }
  }

  if (newAction) {
    newAction->setData(QVariant::fromValue(m_jobs));
    connect(newAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
    result << newAction;
  }

  return result;
}

void KillJobActionFactory::actionTriggered()
{
  QAction *action = qobject_cast<QAction*>(sender());
  if (!action)
    return;

  // The sender was a QAction. Is its data a list of jobs?
  QList<Job> jobs = action->data().value<QList<Job> >();
  if (!jobs.size())
    return;

  QMessageBox::StandardButton confirm =
      QMessageBox::question(NULL, tr("Really cancel jobs?"),
                            tr("Are you sure you would like to cancel %n "
                               "job(s)? ", "", jobs.size()),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No);

  if (confirm != QMessageBox::Yes)
    return;

  foreach (const Job &job, jobs) {
    Queue *queue = m_server->queueManager()->lookupQueue(job.queue());
    if (queue)
      queue->killJob(job);
  }
}

} // namespace MoleQueue
