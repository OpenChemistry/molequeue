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
    m_queueManagerItemModel(new QueueManagerItemModel (m_queueManager, this)),
    m_queueSettingsDialog(NULL)
{
  ui->setupUi(this);

  ui->queueTable->setModel(m_queueManagerItemModel);
  ui->queueTable->horizontalHeader()->setResizeMode(3, QHeaderView::Stretch);

  connect(ui->queueTable, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(doubleClicked(QModelIndex)));
  connect(ui->addQueueButton, SIGNAL(clicked()),
          this, SLOT(addQueue()));
  connect(ui->removeQueueButton, SIGNAL(clicked()),
          this, SLOT(removeQueue()));
  connect(ui->configureQueueButton, SIGNAL(clicked()),
          this, SLOT(configureQueue()));
  connect(ui->closeButton, SIGNAL(clicked()),
          this, SLOT(close()));
  connect(ui->queueTable->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(enableQueueButtons(QItemSelection)));
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
  QList<Queue*> toRemove = getSelectedQueues();
  foreach (Queue* queue, toRemove) {
    m_queueManager->removeQueue(queue);
    queue->deleteLater();
  }
  // reset selection and disable queue buttons
  ui->queueTable->selectionModel()->reset();
  setEnabledQueueButtons(false);
}

void QueueManagerDialog::configureQueue()
{
  QList<Queue*> sel = getSelectedQueues();
  if (!sel.isEmpty())
    showSettingsDialog(sel.first());
}

void QueueManagerDialog::doubleClicked(const QModelIndex &index)
{
  if (index.row() <= m_queueManager->numQueues())
    showSettingsDialog(m_queueManager->queues().at(index.row()));
}

void QueueManagerDialog::showSettingsDialog(Queue *queue)
{
  if (m_queueSettingsDialog) {
    if (m_queueSettingsDialog->currentQueue() == queue) {
      m_queueSettingsDialog->show();
      m_queueSettingsDialog->raise();
      return;
    }

    m_queueSettingsDialog->close();
    m_queueSettingsDialog->deleteLater();
    m_queueSettingsDialog = NULL;
  }

  m_queueSettingsDialog = new QueueSettingsDialog(queue, this);
  m_queueSettingsDialog->show();
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

  foreach (int i, getSelectedRows())
    selectedQueues << allQueues.at(i);

  return selectedQueues;
}

void QueueManagerDialog::setEnabledQueueButtons(bool enabled)
{
  ui->removeQueueButton->setEnabled(enabled);
  ui->configureQueueButton->setEnabled(enabled);
}

void QueueManagerDialog::enableQueueButtons(const QItemSelection &selected)
{
  setEnabledQueueButtons(!selected.isEmpty());
}

} // end MoleQueue namespace
