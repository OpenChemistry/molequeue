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

#include "addqueuedialog.h"
#include "ui_addqueuedialog.h"

#include "queue.h"
#include "queuemanager.h"

#include <QtGui/QMessageBox>
#include <QtGui/QRegExpValidator>

namespace MoleQueue {

AddQueueDialog::AddQueueDialog(QueueManager *queueManager,
                               QWidget *parentObject)
  : QDialog(parentObject),
    ui(new Ui::AddQueueDialog),
    m_queueManager(queueManager)
{
  ui->setupUi(this);

  foreach (const QString &queueName, QueueManager::availableQueues())
    ui->typeComboBox->addItem(queueName);

  // Restrict queue names to alphanumeric strings with internal whitespace
  // (the input is trimmed() in accept()).
  ui->nameLineEdit->setValidator(new QRegExpValidator(
                                   QRegExp(VALID_NAME_REG_EXP)));
}

AddQueueDialog::~AddQueueDialog()
{
  delete ui;
}

void AddQueueDialog::accept()
{
  const QString name = ui->nameLineEdit->text().trimmed();

  if (name.isEmpty()) {
    QMessageBox::critical(this, tr("Missing name"),
                          tr("Please enter a name for the queue before "
                             "continuing."), QMessageBox::Ok);
    return;
  }

  const QString type = ui->typeComboBox->currentText();
  Queue *queue = m_queueManager->addQueue(name, type);

  if (queue) {
    QDialog::accept();
    return;
  }

  // Queue could not be added. Inform user:
  QMessageBox::critical(this, tr("Cannot add queue"),
                        tr("Cannot add queue with queue name '%1', as an "
                           "existing queue already has this name. Please rename"
                           " it and try again.").arg(name));
}

} // end MoleQueue namespace
