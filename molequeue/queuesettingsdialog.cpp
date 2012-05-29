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

#include "queue.h"
#include "queueprogramitemmodel.h"

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

  /// @todo Make these GUI components useful:
  ui->nameLineEdit->setDisabled(true);
  ui->push_addProgram->setDisabled(true);
  ui->push_removeProgram->setDisabled(true);
  ui->push_save->setDisabled(true);
  ui->push_reset->setDisabled(true);
}

QueueSettingsDialog::~QueueSettingsDialog()
{
  delete ui;
}

} // end MoleQueue namespace
