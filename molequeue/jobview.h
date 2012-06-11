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

#ifndef JOBVIEW_H
#define JOBVIEW_H

#include <QtGui/QTableView>

namespace MoleQueue
{
class Job;

class JobView : public QTableView
{
  Q_OBJECT

public:
  JobView(QWidget *theParent = 0);

  /** Custom context menu for this view. */
  void contextMenuEvent(QContextMenuEvent *e);

  QList<int> getSelectedRows();

protected slots:
  void removeSelectedRows();
  void openInAvogadro(const MoleQueue::Job *job = NULL);
  void openInFileBrowser(const MoleQueue::Job *job = NULL);

};

} // End of namespace

#endif
