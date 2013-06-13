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

#include "localqueuewidget.h"
#include "ui_localqueuewidget.h"

#include "queues/local.h"

namespace MoleQueue {

LocalQueueWidget::LocalQueueWidget(QueueLocal *queue, QWidget *parent_) :
  AbstractQueueSettingsWidget(parent_),
  ui(new Ui::LocalQueueWidget),
  m_queue(queue)
{
  ui->setupUi(this);

  reset();

  connect(ui->coresSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(setDirty()));

}

LocalQueueWidget::~LocalQueueWidget()
{
  delete ui;
}

void LocalQueueWidget::save()
{
  m_queue->setMaxNumberOfCores(ui->coresSpinBox->value());
  setDirty(false);
}

void LocalQueueWidget::reset()
{
  ui->coresSpinBox->setValue(m_queue->maxNumberOfCores());
  setDirty(false);
}

} // namespace MoleQueue
