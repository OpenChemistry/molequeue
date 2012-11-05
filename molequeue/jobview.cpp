/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "jobview.h"

#include "actionfactorymanager.h"
#include "job.h"
#include "jobactionfactory.h"
#include "jobitemmodel.h"
#include "jobtableproxymodel.h"

#include <QtGui/QContextMenuEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtCore/QUrl>

namespace MoleQueue
{

JobView::JobView(QWidget *theParent) : QTableView(theParent)
{
}

JobView::~JobView()
{
}

void JobView::contextMenuEvent(QContextMenuEvent *e)
{
  // list of action factories. Map to sort by usefulness
  QMap<unsigned int, JobActionFactory*> factoryMap;
  ActionFactoryManager *manager = ActionFactoryManager::getInstance();
  foreach (JobActionFactory *factory,
           manager->getFactories(JobActionFactory::ContextItem)) {
    factoryMap.insertMulti(factory->usefulness(), factory);
  }

  // Get job under cursor
  Job cursorJob =
      model()->data(indexAt(e->pos()),
                    JobItemModel::FetchJobRole).value<Job>();

  // Get selected jobs
  QList<Job> jobs = selectedJobs();

  QMenu *menu = new QMenu(this);

  // Factories sorted by usefulness:
  QList<JobActionFactory*> factories = factoryMap.values();

  foreach (JobActionFactory *factory, factories) {
    factory->clearJobs();
    // Add all selected jobs if the factory is multijob. Otherwise just the one
    // under the cursor.
    if (factory->isMultiJob()) {
      foreach (const Job &job, jobs)
        factory->addJobIfValid(job);
    }
    else {
      factory->addJobIfValid(cursorJob);
    }

    if (factory->hasValidActions()) {
      if (menu->actions().size())
        menu->addSeparator();
      // Call createActions before menuText, as menuText isn't always static.
      QList<QAction*> actions = factory->createActions();
      QMenu *actionMenu = menu;
      if (factory->useMenu())
        actionMenu = menu->addMenu(factory->menuText());
      foreach (QAction *action, actions) {
        actionMenu->addAction(action);
        action->setParent(actionMenu);
      }
    }
  }

  menu->exec(QCursor::pos());
}

QList<Job> JobView::selectedJobs()
{
  QList<Job> result;

  JobTableProxyModel *proxyModel = qobject_cast<JobTableProxyModel*>(model());

  if (!proxyModel)
    return result;

  JobItemModel *sourceModel =
      qobject_cast<JobItemModel*>(proxyModel->sourceModel());

  if (!sourceModel)
    return result;

  QModelIndexList proxySelection = selectionModel()->selectedRows();

  foreach (const QModelIndex &ind, proxySelection) {
    Job job = sourceModel->data(proxyModel->mapToSource(ind),
                                JobItemModel::FetchJobRole).value<Job>();

    if (job.isValid())
      result << job;

  }

  return result;
}

} // End of namespace
