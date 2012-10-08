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
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <QtCore/QFileInfo>
#include <QtCore/QSettings>

#include "queue.h"
#include "logger.h"
#include "mainwindow.h"
#include "addqueuedialog.h"
#include "importqueuedialog.h"
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
  connect(ui->addQueueButton, SIGNAL(clicked()),
          this, SLOT(addQueue()));
  connect(ui->removeQueueButton, SIGNAL(clicked()),
          this, SLOT(removeQueue()));
  connect(ui->configureQueueButton, SIGNAL(clicked()),
          this, SLOT(configureQueue()));
  connect(ui->importQueueButton, SIGNAL(clicked()),
          this, SLOT(importQueue()));
  connect(ui->exportQueueButton, SIGNAL(clicked()),
          this, SLOT(exportQueue()));
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

void QueueManagerDialog::importQueue()
{
  ImportQueueDialog dialog(m_queueManager, this);
  dialog.exec();
}

void QueueManagerDialog::exportQueue()
{
  // Get selected Queue
  QList<Queue*> selectedQueues = getSelectedQueues();

  // Ensure that only one queue is selected at a time
  if (selectedQueues.size() < 1)
    return;
  if (selectedQueues.size() != 1) {
    QMessageBox::information(this, tr("Queue Export"),
                             tr("Please select only one queue to export at a "
                                "time."), QMessageBox::Ok);
    return;
  }
  Queue *queue = selectedQueues.first();

  // Get initial dir:
  QSettings settings;
  QString initialDir = settings.value("export/queue/lastExportFile",
                                      QDir::homePath()).toString();
  initialDir = QFileInfo(initialDir).dir().absolutePath() +
      QString("/%1.mqq").arg(queue->name());

  // Get filename for export
  QString exportFileName =
      QFileDialog::getSaveFileName(this, tr("Select export filename"),
                                   initialDir,
                                   tr("MoleQueue Queue Export Format (*.mqq);;"
                                      "All files (*)"));

  // User cancel:
  if (exportFileName.isNull())
    return;

  // Set location for next time
  settings.setValue("export/queue/lastExportFile", exportFileName);

  // Prompt whether to export all programs or just the queue details
  QMessageBox::StandardButton exportProgramsButton =
      QMessageBox::question(this, tr("Export programs?"),
                            tr("Would you like to export all program "
                               "configurations along with the queue?\n\n"
                               "Programs: %1")
                            .arg(queue->programNames().join(", ")),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::Yes);

  bool exportPrograms = (exportProgramsButton == QMessageBox::Yes);

  // Populate file
  if (!queue->exportSettings(exportFileName, exportPrograms)) {
    QMessageBox::critical(this, tr("Queue Export"),
                          tr("Could not export queue. Check the log for "
                             "details."), QMessageBox::Ok);
  }
}

void QueueManagerDialog::doubleClicked(const QModelIndex &index)
{
  if (index.row() <= m_queueManager->numQueues())
    showSettingsDialog(m_queueManager->queues().at(index.row()));
}

void QueueManagerDialog::showSettingsDialog(Queue *queue)
{
  QueueSettingsDialog *dialog = NULL;
  // Check if there is already an open dialog for this queue
  dialog = m_queueSettingsDialogs.value(queue, NULL);

  // If not, create one
  if (!dialog) {
    dialog = new QueueSettingsDialog(queue, this);
    m_queueSettingsDialogs.insert(queue, dialog);
    connect(dialog, SIGNAL(finished(int)), this, SLOT(removeSettingsDialog()));
  }

  // Show and raise the dialog
  dialog->show();
  dialog->raise();
}

void QueueManagerDialog::removeSettingsDialog()
{
  QueueSettingsDialog *dialog = qobject_cast<QueueSettingsDialog*>(sender());
  if (!dialog) {
    Logger::logDebugMessage(tr("Internal error in %1: Sender is not a "
                               "QueueSettingsDialog (sender() = %2")
                            .arg(Q_FUNC_INFO)
                            .arg(sender() ? sender()->metaObject()->className()
                                          : "NULL"));
    return;
  }

  m_queueSettingsDialogs.remove(dialog->currentQueue());
  dialog->deleteLater();
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
  ui->exportQueueButton->setEnabled(enabled);
}

void QueueManagerDialog::enableQueueButtons(const QItemSelection &selected)
{
  setEnabledQueueButtons(!selected.isEmpty());
}

} // end MoleQueue namespace
