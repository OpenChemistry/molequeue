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

#include "abstractqueuesettingswidget.h"
#include "logger.h"
#include "importprogramdialog.h"
#include "queue.h"
#include "queuemanager.h"
#include "queueprogramitemmodel.h"
#include "program.h"
#include "programconfiguredialog.h"

#include <QtGui/QCloseEvent>
#include <QtWidgets/QFileDialog>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QMessageBox>
#include <QtGui/QRegExpValidator>
#include <QtWidgets/QHeaderView>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>

namespace MoleQueue {

QueueSettingsDialog::QueueSettingsDialog(Queue *queue, QWidget *parentObject)
  : QDialog(parentObject),
    ui(new Ui::QueueSettingsDialog),
    m_queue(queue),
    m_model(new QueueProgramItemModel (m_queue, this)),
    m_settingsWidget(m_queue->settingsWidget()),
    m_dirty(true)
{
  ui->setupUi(this);

  ui->nameLineEdit->setText(queue->name());
  ui->typeNameLabel->setText(queue->typeName());


  // add queue settings widget
  if (m_settingsWidget) {
    m_settingsWidget->setParent(ui->settingsFrame);
    ui->settingsLayout->addWidget(m_settingsWidget);
    m_settingsWidget->reset();
    connect(m_settingsWidget, SIGNAL(modified()), SLOT(setDirty()));
  }

  // populate programs table
  ui->programsTable->setModel(m_model);
  ui->programsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

  // Make connections
  connect(ui->addProgramButton, SIGNAL(clicked()),
          this, SLOT(addProgramClicked()));
  connect(ui->removeProgramButton, SIGNAL(clicked()),
          this, SLOT(removeProgramClicked()));
  connect(ui->configureProgramButton, SIGNAL(clicked()),
          this, SLOT(configureProgramClicked()));
  connect(ui->importProgramButton, SIGNAL(clicked()),
          this, SLOT(importProgramClicked()));
  connect(ui->exportProgramButton, SIGNAL(clicked()),
          this, SLOT(exportProgramClicked()));
  connect(ui->programsTable, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(doubleClicked(QModelIndex)));
  connect(ui->programsTable->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(enableProgramButtons(QItemSelection)));
  connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
          this, SLOT(buttonBoxButtonClicked(QAbstractButton*)));
  connect(ui->nameLineEdit, SIGNAL(textChanged(QString)), SLOT(setDirty()));
  connect(ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));

  ui->nameLineEdit->setValidator(new QRegExpValidator(
                                   QRegExp(VALID_NAME_REG_EXP)));

  setDirty(false);
}

QueueSettingsDialog::~QueueSettingsDialog()
{
  delete ui;
}

void QueueSettingsDialog::accept()
{
  if (!apply())
    return;

  QDialog::accept();
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

    programAccepted = m_model->addProgram(prog);

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
  QList<Program*> selection = getSelectedPrograms();

  foreach (Program *prog, selection)
    m_model->removeProgram(prog);

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

void QueueSettingsDialog::importProgramClicked()
{
  ImportProgramDialog dialog(m_queue, this);
  dialog.exec();
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

  // Populate file
  program->exportSettings(exportFileName);
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

void QueueSettingsDialog::buttonBoxButtonClicked(QAbstractButton *button)
{
  // "Ok" and "Cancel" are directly connected to accept() and reject(), so only
  // check for "apply" here:
  if (button == ui->buttonBox->button(QDialogButtonBox::Apply))
    apply();
}

bool QueueSettingsDialog::apply()
{
  // If the name changed, check that it won't collide with an existing queue.
  QString name = ui->nameLineEdit->text().trimmed();
  if (name != m_queue->name()) {
    if (QueueManager *queueManager = m_queue->queueManager()) {
      if (queueManager->queueNames().contains(name)) {
        int reply =
            QMessageBox::warning(this, tr("Name conflict"),
                                 tr("The queue name has been changed to '%1', "
                                    "but there is already a queue with that "
                                    "name.\n\nOverwrite existing queue?")
                                 .arg(name), QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No);
        if (reply != QMessageBox::Yes) {
          ui->nameLineEdit->selectAll();
          ui->nameLineEdit->setFocus();
          return false;
        }
      }
      m_queue->setName(name);
    }
  }

  if (m_settingsWidget && m_settingsWidget->isDirty())
    m_settingsWidget->save();

  setDirty(false);
  return true;
}

void QueueSettingsDialog::reset()
{
  ui->nameLineEdit->setText(m_queue->name());
  if (m_settingsWidget)
    m_settingsWidget->reset();
  setDirty(false);
}

void QueueSettingsDialog::setDirty(bool dirty)
{
  if (dirty != m_dirty) {
    m_dirty = dirty;
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(m_dirty);
  }
}

void QueueSettingsDialog::tabChanged(int index)
{
  // We're only interested when the tab changes from settings to programs
  if (index == 0)
    return;

  // Does the configuration need to be saved?
  if (m_dirty) {
    // apply or discard changes?
    QMessageBox::StandardButton reply =
        QMessageBox::warning(this, tr("Unsaved changes"),
                             tr("The changes to the queue have not been saved. "
                                "Would you like to save or discard them?"),
                             QMessageBox::Save | QMessageBox::Discard |
                             QMessageBox::Cancel,
                             QMessageBox::Save);

    switch (reply) {
    case QMessageBox::Cancel:
      ui->tabWidget->setCurrentIndex(0);
      return;
    case QMessageBox::Save:
      apply();
    case QMessageBox::NoButton:
    case QMessageBox::Discard:
    default:
      reset();
      break;
    }
  }
}

void QueueSettingsDialog::closeEvent(QCloseEvent *e)
{
  if (m_dirty) {
    // apply or discard changes?
    QMessageBox::StandardButton reply =
        QMessageBox::warning(this, tr("Unsaved changes"),
                             tr("The changes to the queue have not been saved. "
                                "Would you like to save or discard them?"),
                             QMessageBox::Save | QMessageBox::Discard |
                             QMessageBox::Cancel,
                             QMessageBox::Save);

    switch (reply) {
    case QMessageBox::Cancel:
      e->ignore();
      return;
    case QMessageBox::Save:
      apply();
    case QMessageBox::NoButton:
    case QMessageBox::Discard:
    default:
      reset();
      e->accept();
      break;
    }
  }

  QDialog::closeEvent(e);
}

void QueueSettingsDialog::keyPressEvent(QKeyEvent *e)
{
  // By default, the escape key bypasses the close event, but we still want to
  // check if the settings widget is dirty.
  if (e->key() == Qt::Key_Escape) {
    e->accept();
    close();
    return;
  }

  QDialog::keyPressEvent(e);
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
