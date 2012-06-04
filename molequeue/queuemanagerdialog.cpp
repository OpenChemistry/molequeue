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

#include <QTableWidget>
#include <QTableWidgetItem>

#include "queue.h"
#include "mainwindow.h"
#include "addqueuedialog.h"
#include "queuemanager.h"
#include "queuesettingsdialog.h"

namespace MoleQueue {

QueueManagerDialog::QueueManagerDialog(QueueManager *queueManager,
                                       QWidget *parentObject)
  : QDialog(parentObject),
    ui(new Ui::QueueManagerDialog),
    m_queueManager(queueManager)
{
  ui->setupUi(this);

  // populate queue table
  const QList<Queue *> &queues = queueManager->queues();

  ui->queueTable->setRowCount(queues.size());

  for(int i = 0; i < queues.size(); i++) {
    const Queue *queue = queues[i];

    QTableWidgetItem *nameItem = new QTableWidgetItem(queue->name());
    ui->queueTable->setItem(i, 0, nameItem);
    QTableWidgetItem *typeItem = new QTableWidgetItem(queue->typeName());
    ui->queueTable->setItem(i, 1, typeItem);
  }

  // connect slots
  connect(queueManager, SIGNAL(queueAdded(const MoleQueue::Queue*)),
          this, SLOT(queueAdded(const MoleQueue::Queue*)));
  connect(queueManager, SIGNAL(queueRemoved(const MoleQueue::Queue*)),
          this, SLOT(queueRemoved(const MoleQueue::Queue*)));
  connect(ui->queueTable, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
          this, SLOT(itemDoubleClicked(QTableWidgetItem*)));
  connect(ui->queueTable, SIGNAL(itemClicked(QTableWidgetItem*)),
          this, SLOT(queueSelected(QTableWidgetItem*)));
  connect(ui->addQueueButton, SIGNAL(clicked()), this, SLOT(addQueue()));
  connect(ui->removeQueueButton, SIGNAL(clicked()), this, SLOT(removeQueue()));
  connect(ui->configureQueueButton, SIGNAL(clicked()),
          this, SLOT(configureQueue()));
  connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
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
  int row = ui->queueTable->currentRow();
  Queue *queue = m_queueManager->queues()[row];
  m_queueManager->removeQueue(queue);

  // if we have no queues, disable buttons associate with them
  if (ui->queueTable->rowCount() < 1)
    this->setEnabledQueueButtons(false);
}

void QueueManagerDialog::configureQueue()
{
  int row = ui->queueTable->currentRow();
  Queue *queue = m_queueManager->queues()[row];
  this->showSettingsDialog(queue);
}

void QueueManagerDialog::queueAdded(const MoleQueue::Queue *queue)
{
  int row = ui->queueTable->rowCount();
  ui->queueTable->setRowCount(ui->queueTable->rowCount() + 1);

  QTableWidgetItem *nameItem = new QTableWidgetItem(queue->name());
  ui->queueTable->setItem(row, 0, nameItem);
  QTableWidgetItem *typeItem = new QTableWidgetItem(queue->typeName());
  ui->queueTable->setItem(row, 1, typeItem);
}

void QueueManagerDialog::queueRemoved(const MoleQueue::Queue *queue)
{
  for(int i = 0; i < ui->queueTable->rowCount(); i++){
    QTableWidgetItem *nameItem = ui->queueTable->item(i, 0);
    if(nameItem->text() == queue->name()){
      ui->queueTable->removeRow(i);
    }
  }
}

void QueueManagerDialog::itemDoubleClicked(QTableWidgetItem *item)
{
  int row = item->row();
  Queue *queue = m_queueManager->queues()[row];
  this->showSettingsDialog(queue);
}

void QueueManagerDialog::setEnabledQueueButtons(bool enabled)
{
  ui->removeQueueButton->setEnabled(enabled);
  ui->configureQueueButton->setEnabled(enabled);
}

void QueueManagerDialog::queueSelected(QTableWidgetItem *item)
{
  Q_UNUSED(item)
  this->setEnabledQueueButtons(true);
}

void QueueManagerDialog::showSettingsDialog(Queue *queue)
{
  QueueSettingsDialog dialog(queue, this);
  dialog.exec();
}

} // end MoleQueue namespace
