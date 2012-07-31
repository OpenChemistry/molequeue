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

#include "queuesettingsdialog.h"
#include "ui_queuesettingsdialog.h"

#include "logger.h"
#include "queue.h"
#include "queueprogramitemmodel.h"
#include "program.h"
#include "programconfiguredialog.h"

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QHeaderView>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>

namespace MoleQueue {

QueueSettingsDialog::QueueSettingsDialog(Queue *queue, QWidget *parentObject)
  : QDialog(parentObject),
    ui(new Ui::QueueSettingsDialog),
    m_queue(queue),
    m_model(new QueueProgramItemModel (m_queue, this))
{
  ui->setupUi(this);

  ui->nameLineEdit->setText(queue->name());
  ui->typeNameLabel->setText(queue->typeName());

  // add queue settings widget
  QWidget *settingsWidget = queue->settingsWidget();
  if (settingsWidget) {
    settingsWidget->setParent(ui->settingsFrame);
    ui->settingsLayout->addWidget(settingsWidget);
  }

  // populate programs table
  ui->programsTable->setModel(m_model);
  ui->programsTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);

  // Make connections
  connect(ui->addProgramButton, SIGNAL(clicked()),
          this, SLOT(addProgramClicked()));
  connect(ui->removeProgramButton, SIGNAL(clicked()),
          this, SLOT(removeProgramClicked()));
  connect(ui->configureProgramButton, SIGNAL(clicked()),
          this, SLOT(configureProgramClicked()));
  connect(ui->exportProgramButton, SIGNAL(clicked()),
          this, SLOT(exportProgramClicked()));
  connect(ui->programsTable, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(doubleClicked(QModelIndex)));
  connect(ui->programsTable->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(enableProgramButtons(QItemSelection)));

  // Names can't be changed
  ui->nameLineEdit->setDisabled(true);

  /// @todo Make these GUI components useful:
  ui->push_save->setDisabled(true);
  ui->push_reset->setDisabled(true);
}

QueueSettingsDialog::~QueueSettingsDialog()
{
  delete ui;
}

void QueueSettingsDialog::addProgramClicked()
{
  Program *prog = new Program (m_queue);

  bool programAccepted = false;

  while (!programAccepted) {

    ProgramConfigureDialog configDialog(prog, this);
    DialogCode dialogCode = static_cast<DialogCode>(configDialog.exec());
    if (dialogCode == QDialog::Rejected)
      return;

    programAccepted = m_queue->addProgram(prog, false);

    if (!programAccepted) {
      QMessageBox::information(this, tr("Cannot Add Program"),
                               tr("Cannot add program: Another program with "
                                  "the same name exists. Please enter a "
                                  "different name."));
    }
  }
}

void QueueSettingsDialog::removeProgramClicked()
{
  QModelIndex index = ui->programsTable->currentIndex();
  if (!index.isValid() || index.row() > m_queue->numPrograms())
    return;

  m_model->removeRow(index.row());
  setEnabledProgramButtons(
        !ui->programsTable->selectionModel()->selectedIndexes().isEmpty());
}

void QueueSettingsDialog::configureProgramClicked()
{
  QModelIndex index = ui->programsTable->currentIndex();
  if (!index.isValid() || index.row() > m_queue->numPrograms())
    return;

  showProgramConfigDialog(m_queue->programs().at(index.row()));
}

void QueueSettingsDialog::exportProgramClicked()
{
  // Get selected Program
  QList<Program*> selectedPrograms = this->getSelectedPrograms();

  // Ensure that only one queue is selected at a time
  if (selectedPrograms.size() < 1)
    return;
  if (selectedPrograms.size() != 1) {
    QMessageBox::information(this, tr("Program Export"),
                             tr("Please select only one program to export at a "
                                "time."), QMessageBox::Ok);
    return;
  }
  Program *program= selectedPrograms.first();

  // Get initial dir:
  QSettings settings;
  QString initialDir = settings.value("export/program/lastExportFile",
                                      QDir::homePath()).toString();
  initialDir = QFileInfo(initialDir).dir().absolutePath() +
      QString("/%1-%2.mqp").arg(program->queueName(), program->name());

  // Get filename for export
  QString exportFileName =
      QFileDialog::getSaveFileName(this, tr("Select export filename"),
                                   initialDir,
                                   tr("MoleQueue Program Export Format (*.mqp)"
                                      ";;All files (*)"));

  // User cancel:
  if (exportFileName.isNull())
    return;

  // Set location for next time
  settings.setValue("export/program/lastExportFile", exportFileName);

  // Setup QSettings file to write an INI format file to the filename
  QSettings exporter (exportFileName, QSettings::IniFormat);

  if (!exporter.isWritable()) {
    QMessageBox::critical(this, tr("Cannot export program!"),
                          tr("Cannot export program to file '%1': File is not "
                             "writable.").arg(exportFileName),
                          QMessageBox::Ok);
    return;
  }

  // Populate file
  program->exportConfiguration(exporter);

  // Flush QSettings
  exporter.sync();
}

void QueueSettingsDialog::doubleClicked(const QModelIndex &index)
{
  if (index.isValid() && index.row() <= m_queue->numPrograms())
    showProgramConfigDialog(m_queue->programs().at(index.row()));
}

void QueueSettingsDialog::enableProgramButtons(const QItemSelection &selected)
{
  setEnabledProgramButtons(!selected.isEmpty());
}

QList<int> QueueSettingsDialog::getSelectedRows()
{
  QItemSelection sel (ui->programsTable->selectionModel()->selection());

  QList<int> rows;
  foreach (const QModelIndex &ind, sel.indexes()) {
    if (!rows.contains(ind.row()))
      rows << ind.row();
  }

  qSort(rows);
  return rows;
}

QList<Program *> QueueSettingsDialog::getSelectedPrograms()
{
  QList<Program *> allPrograms = m_queue->programs();
  QList<Program *> selectedPrograms;

  foreach (int i, getSelectedRows())
    selectedPrograms << allPrograms.at(i);

  return selectedPrograms;
}

void QueueSettingsDialog::removeProgramDialog()
{
  ProgramConfigureDialog *dialog =
      qobject_cast<ProgramConfigureDialog*>(sender());
  if (!dialog) {
    Logger::logDebugMessage(tr("Internal error in %1: Sender is not a "
                               "ProgramConfigureDialog (sender() = %2")
                            .arg(Q_FUNC_INFO)
                            .arg(sender() ? sender()->metaObject()->className()
                                          : "NULL"));
    return;
  }

  m_programConfigureDialogs.remove(dialog->currentProgram());
  dialog->deleteLater();
}

void QueueSettingsDialog::showProgramConfigDialog(Program *prog)
{
  ProgramConfigureDialog *dialog = NULL;
  // Check if there is already an open dialog for this queue
  dialog = m_programConfigureDialogs.value(prog, NULL);

  // If not, create one
  if (!dialog) {
    dialog = new ProgramConfigureDialog(prog, this);
    m_programConfigureDialogs.insert(prog, dialog);
    connect(dialog, SIGNAL(finished(int)), this, SLOT(removeProgramDialog()));
  }

  // Show and raise the dialog
  dialog->show();
  dialog->raise();
}

void QueueSettingsDialog::setEnabledProgramButtons(bool enabled)
{
  ui->removeProgramButton->setEnabled(enabled);
  ui->configureProgramButton->setEnabled(enabled);
  ui->exportProgramButton->setEnabled(enabled);
}

} // end MoleQueue namespace
