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

#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>

#include <QtCore/QProcess>
#include <QtCore/QDebug>

namespace MoleQueue
{

JobView::JobView(QWidget *theParent) : QTableView(theParent)
{
}

void JobView::contextMenuEvent(QContextMenuEvent *e)
{
  Q_UNUSED(e);
  /*
  QModelIndex index = indexAt(e->pos());
  if (index.isValid()) {
    Job *job = static_cast<Job *>(index.internalPointer());
    QMenu *menu = new QMenu(this);

    if (job->jobState() == MoleQueue::Finished) {
      QAction *action = menu->addAction("&Open in Avogadro");
      action->setData(QVariant(job->outputFile()));
      connect(action, SIGNAL(triggered()), this, SLOT(openInAvogadro()));
    }
    menu->exec(QCursor::pos());
  }
  */
}

void JobView::openInAvogadro()
{
  QAction *action = static_cast<QAction*>(sender());
  if (action) {
    qDebug() << "Open in avogadro..." << action->data();
    QProcess::startDetached("/home/marcus/ssd/build/avogadro-squared/prefix/bin/avogadro " + action->data().toString());
  }
}

} // End of namespace
