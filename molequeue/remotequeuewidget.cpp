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

#include "remotequeuewidget.h"
#include "ui_remotequeuewidget.h"

#include "queues/remote.h"

#include <QtGui/QTextDocument>

namespace MoleQueue
{

RemoteQueueWidget::RemoteQueueWidget(QueueRemote *queue,
                                     QWidget *parentObject) :
  QWidget(parentObject),
  ui(new Ui::RemoteQueueWidget),
  m_queue(queue)
{
  ui->setupUi(this);

  this->updateGuiFromQueue();

  connect(ui->edit_submissionCommand, SIGNAL(textChanged(QString)),
          this, SLOT(updateSubmissionCommand(QString)));
  connect(ui->edit_requestQueueCommand, SIGNAL(textChanged(QString)),
          this, SLOT(updateRequestQueueCommand(QString)));
  connect(ui->edit_launchScriptName, SIGNAL(textChanged(QString)),
          this, SLOT(updateLaunchScriptName(QString)));
  connect(ui->edit_workingDirectoryBase, SIGNAL(textChanged(QString)),
          this, SLOT(updateWorkingDirectoryBase(QString)));
  connect(ui->edit_hostName, SIGNAL(textChanged(QString)),
          this, SLOT(updateHostName(QString)));
  connect(ui->edit_userName, SIGNAL(textChanged(QString)),
          this, SLOT(updateUserName(QString)));
  connect(ui->spin_sshPort, SIGNAL(valueChanged(int)),
          this, SLOT(updateSshPort(int)));
  connect(ui->text_launchTemplate, SIGNAL(textChanged()),
          this, SLOT(updateLaunchTemplate()));
}

RemoteQueueWidget::~RemoteQueueWidget()
{
  delete ui;
}

void RemoteQueueWidget::updateGuiFromQueue()
{
  ui->edit_submissionCommand->setText(m_queue->submissionCommand());
  ui->edit_requestQueueCommand->setText(m_queue->requestQueueCommand());
  ui->edit_launchScriptName->setText(m_queue->launchScriptName());
  ui->edit_workingDirectoryBase->setText(m_queue->workingDirectoryBase());
  ui->edit_hostName->setText(m_queue->hostName());
  ui->edit_userName->setText(m_queue->userName());
  ui->spin_sshPort->setValue(m_queue->sshPort());
  ui->text_launchTemplate->document()->setPlainText(m_queue->launchTemplate());
}

void RemoteQueueWidget::updateSubmissionCommand(const QString &command)
{
  m_queue->setSubmissionCommand(command);
}

void RemoteQueueWidget::updateRequestQueueCommand(const QString &command)
{
  m_queue->setRequestQueueCommand(command);
}

void RemoteQueueWidget::updateLaunchScriptName(const QString &name)
{
  m_queue->setLaunchScriptName(name);
}

void RemoteQueueWidget::updateWorkingDirectoryBase(const QString &dir)
{
  m_queue->setWorkingDirectoryBase(dir);
}

void RemoteQueueWidget::updateHostName(const QString &hostName)
{
  m_queue->setHostName(hostName);
}

void RemoteQueueWidget::updateUserName(const QString &userName)
{
  m_queue->setUserName(userName);
}

void RemoteQueueWidget::updateSshPort(int sshPort)
{
  m_queue->setSshPort(sshPort);
}

void RemoteQueueWidget::updateLaunchTemplate()
{
  QString text = ui->text_launchTemplate->document()->toPlainText();
  m_queue->setLaunchTemplate(text);
}

} // end namespace MoleQueue
