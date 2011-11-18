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

namespace MoleQueue {

QueueManagerDialog::QueueManagerDialog(QueueManager *queueManager, QWidget *parent)
  : QDialog(parent),
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
  connect(queueManager, SIGNAL(queueAdded(const MoleQueue::Queue*)), this, SLOT(queueAdded(const MoleQueue::Queue*)));
  connect(queueManager, SIGNAL(queueRemoved(const MoleQueue::Queue*)), this, SLOT(queueRemoved(const MoleQueue::Queue*)));
  connect(ui->addQueueButton, SIGNAL(clicked()), this, SLOT(addQueue()));
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

} // end MoleQueue namespace
