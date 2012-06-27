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

#include "job.h"
#include "jobitemmodel.h"

#include <QtGui/QContextMenuEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtCore/QUrl>

Q_DECLARE_METATYPE(QList<int>)

namespace MoleQueue
{

JobView::JobView(QWidget *theParent) : QTableView(theParent)
{
  qRegisterMetaType<QList<int> >("QList<int>");
}

void JobView::contextMenuEvent(QContextMenuEvent *e)
{
  QModelIndex index = indexAt(e->pos());
  const Job *job =
      this->model()->data(index, JobItemModel::FetchJobRole).value<const Job*>();
  QList<int> selectedRows = this->getSelectedRows();
  int numSelectedRows = selectedRows.size();

  if (index.isValid()) {
    QMenu *menu = new QMenu(this);

    if (numSelectedRows > 0) {
      QAction *action =
          menu->addAction(tr("&Remove %n job(s)...", "", numSelectedRows));
      action->setData(QVariant::fromValue(selectedRows));
      connect(action, SIGNAL(triggered()), this, SLOT(removeSelectedRows()));
    }

    menu->addSeparator();

    if (job) {
      // Open in file browser
      QAction *action = menu->addAction(tr("View files for '%1'...")
                               .arg(job->description()));
      action->setData(QVariant::fromValue(job));
      connect(action, SIGNAL(triggered()), this, SLOT(openInFileBrowser()));

      // Open in avogadro
      action = menu->addAction(tr("Open '%1' in &Avogadro...")
                                        .arg(job->description()));
      action->setData(QVariant::fromValue(job));
      connect(action, SIGNAL(triggered()), this, SLOT(openInAvogadro()));
     }
    if (numSelectedRows > 1) {
      // Open in file browser
      QAction *action = menu->addAction(tr("View files for %n job(s)...",
                                           "", numSelectedRows));
      action->setData(QVariant::fromValue(selectedRows));
      connect(action, SIGNAL(triggered()), this, SLOT(openInFileBrowser()));

      // Open in avogadro
      action = menu->addAction(tr("Open %n job(s) in A&vogadro...",
                                           "", numSelectedRows));
      action->setData(QVariant::fromValue(selectedRows));
      connect(action, SIGNAL(triggered()), this, SLOT(openInAvogadro()));
    }

    menu->exec(QCursor::pos());
  }
}

QList<int> JobView::getSelectedRows()
{
  QItemSelection sel (this->selectionModel()->selection());

  QList<int> rows;
  foreach (const QModelIndex &ind, sel.indexes()) {
    if (!rows.contains(ind.row()))
      rows << ind.row();
  }

  qSort(rows);
  return rows;
}

void JobView::removeSelectedRows()
{
  QAction *action = qobject_cast<QAction*>(this->sender());
  if (!action)
    return;

  QList<int> sel = action->data().value<QList<int> >();

  QMessageBox::StandardButton confirm =
      QMessageBox::question(this, tr("Really remove jobs?"),
                            tr("Are you sure you would like to remove %n job(s)? "
                               "This will not delete any input or output files.",
                               "", sel.size()),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (confirm != QMessageBox::Yes)
    return;

  // Sel is sorted, delete rows in reverse order
  for (QList<int>::const_iterator it = sel.constEnd() - 1,
       it_end = sel.constBegin(); it >= it_end; --it) {
    this->model()->removeRow(*it);
  }
}

void JobView::openInAvogadro(const Job *job)
{
  // If no job was passed, check that the sender is a QAction.
  if (job == NULL) {
    QAction *action = qobject_cast<QAction*>(this->sender());
    if (!action)
      return;

    // The sender was a QAction. Is its data a list of rows, or a specific job?
    QList<int> rows = action->data().value<QList<int> >();
    if (rows.size()) {
      // It's a list of rows -- open each row's job in avogadro.
      foreach (int row, rows) {
        const Job *dataJob =
            this->model()->data(this->model()->index(row, 0),
                                JobItemModel::FetchJobRole).value<const Job*>();
        if (dataJob)
          this->openInAvogadro(dataJob);
      }
    }
    else {
      // The data is not a list of rows. Is is a pointer to a job?
      const Job *dataJob = action->data().value<const Job*>();
      if (dataJob)
        this->openInAvogadro(dataJob);
    }
    return;
  }

  // We have a job -- open it.
  /// @todo Find a better way to locate Avogadro, extend to other programs.
  // find queue
  QProcess::startDetached(QString("/ssd/src/avogadro/build/bin/avogadro %1/%2")
                          .arg(job->localWorkingDirectory(), "job.out"));

}

void JobView::openInFileBrowser(const Job *job)
{
  // If no job was passed, check that the sender is a QAction.
  if (job == NULL) {
    QAction *action = qobject_cast<QAction*>(this->sender());
    if (!action)
      return;

    // The sender was a QAction. Is its data a list of rows, or a specific job?
    QList<int> rows = action->data().value<QList<int> >();
    if (rows.size()) {
      // It's a list of rows -- open each row's job in avogadro.
      foreach (int row, rows) {
        const Job *dataJob =
            this->model()->data(this->model()->index(row, 0),
                                JobItemModel::FetchJobRole).value<const Job*>();
        if (dataJob)
          this->openInFileBrowser(dataJob);
      }
    }
    else {
      // The data is not a list of rows. Is is a pointer to a job?
      const Job *dataJob = action->data().value<const Job*>();
      if (dataJob)
        this->openInFileBrowser(dataJob);
    }
    return;
  }

  // We have a job -- open it.
  QDesktopServices::openUrl(QUrl::fromLocalFile(job->localWorkingDirectory()));
}

} // End of namespace
