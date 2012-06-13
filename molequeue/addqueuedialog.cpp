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

#include "addqueuedialog.h"
#include "ui_addqueuedialog.h"

#include "queue.h"
#include "queuemanager.h"
#include "queues/local.h"
#include "queues/pbs.h"
#include "queues/remote.h"
#include "queues/sge.h"

namespace MoleQueue {

AddQueueDialog::AddQueueDialog(QueueManager *queueManager,
                               QWidget *parentObject)
  : QDialog(parentObject),
    ui(new Ui::AddQueueDialog),
    m_queueManager(queueManager)
{
    ui->setupUi(this);

    /// @todo Find a more scalable way to handle this...
    ui->typeComboBox->addItem(tr("Local"));
    ui->typeComboBox->addItem(tr("Sun Grid Engine"));
    ui->typeComboBox->addItem(tr("PBS/Torque"));

    connect(this, SIGNAL(accepted()), SLOT(addQueue()));
}

AddQueueDialog::~AddQueueDialog()
{
    delete ui;
}

void AddQueueDialog::addQueue()
{
  const QString queueType = ui->typeComboBox->currentText();
  /// @todo Find a more scalable way to handle this...
  Queue *queue = NULL;
  if (queueType == tr("Local"))
    queue = new QueueLocal (m_queueManager);
  else if (queueType == tr("Sun Grid Engine"))
    queue = new QueueSge (m_queueManager);
  else if (queueType == tr("PBS/Torque"))
    queue = new QueuePbs (m_queueManager);

  if(queue){
    queue->setName(ui->nameLineEdit->text());
    m_queueManager->addQueue(queue);
  }
}

} // end MoleQueue namespace
