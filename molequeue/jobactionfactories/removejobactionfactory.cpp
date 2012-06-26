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

#include "removejobactionfactory.h"

#include "../job.h"
#include "../jobmanager.h"
#include "../server.h"

#include <QtGui/QAction>
#include <QtGui/QMessageBox>

Q_DECLARE_METATYPE(QList<const MoleQueue::Job*>)

namespace MoleQueue {

RemoveJobActionFactory::RemoveJobActionFactory() :
  JobActionFactory()
{
  qRegisterMetaType<QList<const Job*> >("QList<const Job*>");
  m_isMultiJob = true;
  m_flags |= JobActionFactory::ContextItem;
}

RemoveJobActionFactory::~RemoveJobActionFactory()
{
}

bool RemoveJobActionFactory::isValidForJob(const Job *job) const
{
  return m_server->jobManager()->jobs().contains(job);
}

QList<QAction *> RemoveJobActionFactory::createActions()
{
  QList<QAction*> result;

  if (m_attemptedJobAdditions == 1) {
    QAction *newAction = new QAction (
          tr("Remove '%1'...").arg(m_jobs.first()->description()), NULL);
    newAction->setData(QVariant::fromValue(m_jobs));
    connect(newAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
    result << newAction;
  }
  else if (m_attemptedJobAdditions > 1) {
    QAction *newAction = new QAction (NULL);
    if (static_cast<unsigned int>(m_jobs.size()) == m_attemptedJobAdditions)
      newAction->setText(tr("Remove %1 jobs...").arg(m_jobs.size()));
    else
      newAction->setText(tr("Remove %1 of %2 selected jobs...")
                         .arg(m_jobs.size()).arg(m_attemptedJobAdditions));
    newAction->setData(QVariant::fromValue(m_jobs));
    connect(newAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
    result << newAction;
  }

  return result;
}

void RemoveJobActionFactory::actionTriggered()
{
  QAction *action = qobject_cast<QAction*>(this->sender());
  if (!action)
    return;

  // The sender was a QAction. Is its data a list of jobs?
  QList<const Job *> jobs = action->data().value<QList<const Job*> >();
  if (!jobs.size())
    return;

  QMessageBox::StandardButton confirm =
      QMessageBox::question(NULL, tr("Really remove jobs?"),
                            tr("Are you sure you would like to remove %n job(s)? "
                               "This will not delete any input or output files.",
                               "", jobs.size()),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (confirm != QMessageBox::Yes)
    return;

  m_server->jobManager()->removeJobs(jobs);
}



} // namespace MoleQueue
