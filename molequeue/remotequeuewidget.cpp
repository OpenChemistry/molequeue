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

#include "client.h"
#include "program.h"
#include "queues/remote.h"
#include "sshcommand.h"

#include <QtCore/QTimer>

#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>
#include <QtGui/QTextDocument>

namespace MoleQueue
{

RemoteQueueWidget::RemoteQueueWidget(QueueRemote *queue,
                                     QWidget *parentObject) :
  QWidget(parentObject),
  ui(new Ui::RemoteQueueWidget),
  m_queue(queue),
  m_client(NULL)
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

  connect(ui->push_testConnection, SIGNAL(clicked()),
          this, SLOT(testConnection()));
  connect(ui->push_sleepTest, SIGNAL(clicked()),
          this, SLOT(sleepTest()));
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

void RemoteQueueWidget::testConnection()
{
  // Verify information
  QString host = ui->edit_hostName->text();
  QString user = ui->edit_userName->text();
  int port = ui->spin_sshPort->value();

  if (host.isEmpty() || user.isEmpty()) {
    QMessageBox::warning(this, tr("Cannot connect to remote host."),
                         tr("Cannot connect to remote host: invalid host "
                            "specification: %1@%2").arg(host, user));
    return;
  }

  // Create SSH connection
  SshConnection *conn = new SshCommand (this);
  conn->setHostName(host);
  conn->setUserName(user);
  conn->setPortNumber(port);

  // Create ProgressDialog
  QProgressDialog *prog = new QProgressDialog (this);
  prog->setWindowTitle(tr("Testing remote connection..."));
  prog->setLabelText(tr("Attempting to connect to %1@%2:%3...")
                     .arg(user).arg(host).arg(port));
  prog->setMinimumDuration(0);
  prog->setWindowModality(Qt::WindowModal);
  prog->setRange(0, 0);
  prog->setValue(0);

  QTimer *timeout = new QTimer (this);
  connect(conn, SIGNAL(requestComplete()),
          prog, SLOT(accept()));
  connect(timeout, SIGNAL(timeout()),
          prog, SLOT(reject()));

  // Wait 15 seconds for timeout
  timeout->start(15000);
  conn->execute("echo ok");
  prog->exec();
  prog->hide();

  if (prog->wasCanceled()) {
    conn->deleteLater();
    prog->deleteLater();
    return;
  }

  if (prog->result() == QProgressDialog::Rejected) {
    QMessageBox::critical(this, tr("Connection timeout"),
                          tr("The connection to %1@%2:%3 failed: connection"
                             " timed out.").arg(user).arg(host).arg(port));
    conn->deleteLater();
    prog->deleteLater();
    return;
  }

  prog->hide();
  prog->deleteLater();

  // Verify output and exit code
  if (conn->exitCode() != 0 ||
      conn->output().trimmed() != "ok") {
    QMessageBox::critical(this, tr("SSH Error"),
                          tr("The connection to %1@%2:%3 failed: "
                             "exit code: %4. Output:\n\n%5")
                          .arg(user).arg(host).arg(port)
                          .arg(conn->exitCode()).arg(conn->output()));
    conn->deleteLater();
    return;
  }

  QMessageBox::information(this, tr("Success"),
                           tr("SSH connection to %1@%2:%3 succeeded!")
                           .arg(user).arg(host).arg(port));
  conn->deleteLater();

  return;
}

void RemoteQueueWidget::sleepTest()
{
  // Check that important variables are set:
  QString missingVariable = "";
  if (m_queue->hostName().isEmpty())
    missingVariable = tr("server hostname");
  else if (m_queue->userName().isEmpty())
    missingVariable = tr("server username");
  else if (m_queue->submissionCommand().isEmpty())
    missingVariable = tr("job submission command");
  else if (m_queue->requestQueueCommand().isEmpty())
    missingVariable = tr("queue request command");
  else if (m_queue->workingDirectoryBase().isEmpty())
    missingVariable = tr("remote working directory");

  if (!missingVariable.isEmpty()) {
    QMessageBox::critical(this, tr("Missing information"),
                          tr("Refusing to test job submission: %1 not set.")
                          .arg(missingVariable));
    return;
  }

  Program *sleepProgram = m_queue->lookupProgram("sleep (testing)");

  if (sleepProgram == NULL) {
    // Add sleep if it's not present
    sleepProgram = new Program (m_queue);
    sleepProgram->setName("sleep (testing)");
    sleepProgram->setArguments("30");
    sleepProgram->setExecutable("sleep");
    sleepProgram->setUseExecutablePath(false);
    sleepProgram->setInputFilename("");
    sleepProgram->setOutputFilename("");
    sleepProgram->setLaunchSyntax(Program::PLAIN);
    m_queue->addProgram(sleepProgram);
  }

  if (!m_client) {
    m_client = new Client (this);
    m_client->connectToServer();
  }

  Job *sleepJob = m_client->newJobRequest();
  sleepJob->setQueue(m_queue->name());
  sleepJob->setProgram(sleepProgram->name());
  sleepJob->setDescription("sleep 30 (test)");

  m_client->submitJobRequest(sleepJob);
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
