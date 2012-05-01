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

#ifndef QUEUEMANAGERDIALOG_H
#define QUEUEMANAGERDIALOG_H

#include <QDialog>
#include <QTableWidget>

namespace Ui {
class QueueManagerDialog;
}

namespace MoleQueue {

class Queue;
class QueueManager;

class QueueManagerDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QueueManagerDialog(QueueManager *manager, QWidget *parent = 0);
  ~QueueManagerDialog();

private slots:
  void addQueue();
  void removeQueue();
  void queueAdded(const MoleQueue::Queue *queue);
  void queueRemoved(const MoleQueue::Queue *queue);
  void itemDoubleClicked(QTableWidgetItem *item);

private:
  Ui::QueueManagerDialog *ui;
  QueueManager *m_queueManager;
};

} // end MoleQueue namespace

#endif // QUEUEMANAGERDIALOG_H
