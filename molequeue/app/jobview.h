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

#ifndef MOLEQUEUE_JOBVIEW_H
#define MOLEQUEUE_JOBVIEW_H

#include <QtWidgets/QTableView>

namespace MoleQueue
{
class Job;

/// MVC item view for the job table.
class JobView : public QTableView
{
  Q_OBJECT

public:
  JobView(QWidget *theParent = 0);
  ~JobView();

  /** Custom context menu for this view. */
  void contextMenuEvent(QContextMenuEvent *e);

  QList<Job> selectedJobs();
};

} // End of namespace

#endif
