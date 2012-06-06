/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

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
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

#include <QtCore/QProcess>
#include <QtCore/QDebug>

Q_DECLARE_METATYPE(QList<int>)

namespace MoleQueue
{

JobView::JobView(QWidget *theParent) : QTableView(theParent)
{
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


} // End of namespace
