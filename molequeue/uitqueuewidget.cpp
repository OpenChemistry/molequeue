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

#include "uitqueuewidget.h"
#include "ui_uitqueuewidget.h"

#include "transport/localsocket/localsocketclient.h"
#include "program.h"
#include "queues/uit/queueuit.h"
#include "templatekeyworddialog.h"

#include <QtCore/QTimer>
#include <QtCore/QSettings>

#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>
#include <QtGui/QTextDocument>
#include <QtGui/QFileDialog>

namespace MoleQueue
{

UitQueueWidget::UitQueueWidget(QueueUit *queue,
                               QWidget *parentObject)
  : AbstractQueueSettingsWidget(parentObject),
    ui(new Ui::UitQueueWidget), m_queue(queue),
    m_client(NULL), m_helpDialog(NULL)
{
  ui->setupUi(this);

  reset();

  connect(ui->editWorkingDirectoryBase, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->editHostName, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->editKerberosPrinciple, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->wallTimeHours, SIGNAL(valueChanged(int)),
          this, SLOT(setDirty()));
  connect(ui->wallTimeMinutes, SIGNAL(valueChanged(int)),
          this, SLOT(setDirty()));

  connect(ui->pushTestConnection, SIGNAL(clicked()),
          this, SLOT(testConnection()));
  connect(ui->pushSleepTest, SIGNAL(clicked()),
          this, SLOT(sleepTest()));
  connect(ui->templateHelpButton, SIGNAL(clicked()),
          this, SLOT(showHelpDialog()));
}

UitQueueWidget::~UitQueueWidget()
{
  delete ui;
}

void UitQueueWidget::save()
{
  //m_queue->setWorkingDirectoryBase(ui->editWorkingDirectoryBase->text());
  m_queue->setHostName(ui->editHostName->text());
  m_queue->setKerberosPrinciple(ui->editKerberosPrinciple->text());
  //m_queue->setQueueUpdateInterval(ui->updateIntervalSpin->value());

  QString text = ui->text_launchTemplate->document()->toPlainText();
  m_queue->setLaunchTemplate(text);

  int hours = ui->wallTimeHours->value();
  int minutes = ui->wallTimeMinutes->value();
  m_queue->setDefaultMaxWallTime(minutes + (hours * 60));
  setDirty(false);
}

void UitQueueWidget::reset()
{
  ui->editHostName->setText(m_queue->hostName());
  ui->editKerberosPrinciple->setText(m_queue->kerberosPrinciple());
  setDirty(false);
}

void UitQueueWidget::testConnection()
{
  m_queue->testConnection(this);
}

void UitQueueWidget::sleepTest()
{
  // TODO
}

void UitQueueWidget::showHelpDialog()
{
  if (!m_helpDialog)
    m_helpDialog = new TemplateKeywordDialog(this);
  m_helpDialog->show();
}

} // end namespace MoleQueue
