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

#ifndef MOLEQUEUE_ADVANCEDFILTERDIALOG_H
#define MOLEQUEUE_ADVANCEDFILTERDIALOG_H

#include <QtWidgets/QDialog>

namespace Ui {
class AdvancedFilterDialog;
}

namespace MoleQueue {
class JobTableProxyModel;

/// @brief Provides advanced filtering options for the JobView.
class AdvancedFilterDialog : public QDialog
{
  Q_OBJECT

public:
  AdvancedFilterDialog(JobTableProxyModel *model, QWidget *par = 0);
  ~AdvancedFilterDialog();

protected slots:
  void updateFilters();
  void selectAllStatuses();
  void selectNoStatuses();

protected:
  Ui::AdvancedFilterDialog *ui;
  JobTableProxyModel *m_proxyModel;
};

} // namespace MoleQueue
#endif // MOLEQUEUE_ADVANCEDFILTERDIALOG_H
