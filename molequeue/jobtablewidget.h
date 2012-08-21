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

#ifndef JOBTABLEWIDGET_H
#define JOBTABLEWIDGET_H

#include "molequeueglobal.h"

#include <QtGui/QWidget>

namespace Ui {
class JobTableWidget;
}

namespace MoleQueue
{
class Job;
class JobActionFactory;
class JobManager;
class JobTableProxyModel;

/// @brief Widget which encapsulates the Job table MVC classes.
class JobTableWidget : public QWidget
{
  Q_OBJECT

public:
  explicit JobTableWidget(QWidget *parentObject = 0);
  ~JobTableWidget();

  void setJobManager(JobManager *jobManager);
  JobManager * jobManager() const { return m_jobManager; }

signals:
  void jobCountsChanged(int totalJobs, int shownJobs);

public slots:
  void clearFinishedJobs();
  void showFilterBar(bool visible = true);
  void hideFilterBar() { showFilterBar(false); }
  void focusInFilter();

protected slots:
  void saveUiState();
  void restoreUiState();

  void blockFilterUiSignals(bool block = true);
  void updateFilters();

  void selectAllStatuses();
  void selectNoStatuses();

  void modelRowCountChanged();

protected:
  // Row indices, ascending order
  QList<int> getSelectedRows();

  Ui::JobTableWidget *ui;
  JobManager *m_jobManager;
  JobTableProxyModel *m_proxyModel;
};

} // end namespace MoleQueue

#endif // JOBTABLEWIDGET_H
