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

#include <QtGui/QMessageBox>
#include <QtGui/QHeaderView>

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
  connect(ui->push_addProgram, SIGNAL(clicked()),
          this, SLOT(addProgramClicked()));
  connect(ui->programsTable, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(doubleClicked(QModelIndex)));

  /// @todo Make these GUI components useful:
  ui->nameLineEdit->setDisabled(true);
  ui->push_removeProgram->setDisabled(true);
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

void QueueSettingsDialog::doubleClicked(const QModelIndex &index)
{
  if (index.row() <= m_queue->numPrograms())
    showProgramConfigDialog(m_queue->programs().at(index.row()));
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

} // end MoleQueue namespace
