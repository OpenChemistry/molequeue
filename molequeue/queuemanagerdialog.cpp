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

#include "queuemanagerdialog.h"
#include "ui_queuemanagerdialog.h"

#include <QtGui/QItemSelection>

#include "queue.h"
#include "mainwindow.h"
#include "addqueuedialog.h"
#include "queuemanager.h"
#include "queuemanageritemmodel.h"
#include "queuesettingsdialog.h"

namespace MoleQueue {

QueueManagerDialog::QueueManagerDialog(QueueManager *queueManager,
                                       QWidget *parentObject)
  : QDialog(parentObject),
    ui(new Ui::QueueManagerDialog),
    m_queueManager(queueManager),
    m_queueManagerItemModel(new QueueManagerItemModel (m_queueManager, this))
{
  ui->setupUi(this);

  ui->queueTable->setModel(m_queueManagerItemModel);
  ui->queueTable->horizontalHeader()->setResizeMode(3, QHeaderView::Stretch);

  connect(ui->queueTable, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(doubleClicked(QModelIndex)));
  connect(m_queueManagerItemModel, SIGNAL(layoutChanged()),
          this, SLOT(modelLayoutChanged()));
  connect(ui->addQueueButton, SIGNAL(clicked()),
          this, SLOT(addQueue()));
  connect(ui->removeQueueButton, SIGNAL(clicked()),
          this, SLOT(removeQueue()));
  connect(ui->configureQueueButton, SIGNAL(clicked()),
          this, SLOT(configureQueue()));
  connect(ui->closeButton, SIGNAL(clicked()),
          this, SLOT(close()));
}

QueueManagerDialog::~QueueManagerDialog()
{
  delete ui;
}

void QueueManagerDialog::addQueue()
{
  AddQueueDialog dialog(m_queueManager, this);
  dialog.exec();
}

void QueueManagerDialog::removeQueue()
{
  QList<Queue*> toRemove = this->getSelectedQueues();
  foreach (Queue* queue, toRemove) {
    m_queueManager->removeQueue(queue);
    queue->deleteLater();
  }
}

void QueueManagerDialog::configureQueue()
{
  QList<Queue*> sel = this->getSelectedQueues();
  if (!sel.isEmpty())
    this->showSettingsDialog(sel.first());
}

void QueueManagerDialog::doubleClicked(const QModelIndex &index)
{
  if (index.row() <= m_queueManager->numQueues())
    this->showSettingsDialog(m_queueManager->queues().at(index.row()));
}

void QueueManagerDialog::showSettingsDialog(Queue *queue)
{
  QueueSettingsDialog dialog(queue, this);
  dialog.exec();
}

void QueueManagerDialog::modelLayoutChanged()
{
  // Enable/disable relevant buttons
  int rows = m_queueManagerItemModel->rowCount();

  if (rows == 0) {
    ui->configureQueueButton->setDisabled(true);
    ui->removeQueueButton->setDisabled(true);
  }
  else {
    ui->configureQueueButton->setEnabled(true);
    ui->removeQueueButton->setEnabled(true);
  }
}

QList<int> QueueManagerDialog::getSelectedRows()
{
  QItemSelection sel (ui->queueTable->selectionModel()->selection());

  QList<int> rows;
  foreach (const QModelIndex &ind, sel.indexes()) {
    if (!rows.contains(ind.row()))
      rows << ind.row();
  }

  qSort(rows);
  return rows;
}

QList<Queue *> QueueManagerDialog::getSelectedQueues()
{
  QList<Queue *> allQueues = m_queueManager->queues();
  QList<Queue *> selectedQueues;

  foreach (int i, this->getSelectedRows())
    selectedQueues << allQueues.at(i);

  return selectedQueues;
}

} // end MoleQueue namespace
