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

#ifndef QUEUEMANAGERDIALOG_H
#define QUEUEMANAGERDIALOG_H

#include <QDialog>
#include <QTableWidget>

namespace Ui {
class QueueManagerDialog;
}

namespace MoleQueue
{
class Queue;
class QueueManager;
class QueueManagerItemModel;
class QueueSettingsDialog;

/// @brief Dialog for managing supported queues.
class QueueManagerDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QueueManagerDialog(QueueManager *manager, QWidget *parentObject = 0);
  ~QueueManagerDialog();

protected slots:
  void addQueue();
  void removeQueue();
  void configureQueue();
  void importQueue();
  void exportQueue();
  void doubleClicked(const QModelIndex &);
  void showSettingsDialog(MoleQueue::Queue *queue);
  void removeSettingsDialog();
  void enableQueueButtons(const QItemSelection &selected);

protected:
  /// Row indices, ascending order
  QList<int> getSelectedRows();
  QList<Queue*> getSelectedQueues();
  void setEnabledQueueButtons(bool enabled);

  Ui::QueueManagerDialog *ui;
  QueueManager *m_queueManager;
  QueueManagerItemModel *m_queueManagerItemModel;
  QMap<Queue *, QueueSettingsDialog *> m_queueSettingsDialogs;
};

} // end MoleQueue namespace

#endif // QUEUEMANAGERDIALOG_H
