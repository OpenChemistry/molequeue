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

#include "transport/localsocket/localsocketclient.h"
#include "program.h"
#include "queues/remote.h"
#include "sshcommand.h"
#include "templatekeyworddialog.h"

#include <QtCore/QTimer>

#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>
#include <QtGui/QTextDocument>

namespace MoleQueue
{

RemoteQueueWidget::RemoteQueueWidget(QueueRemote *queue,
                                     QWidget *parentObject) :
  AbstractQueueSettingsWidget(parentObject),
  ui(new Ui::RemoteQueueWidget),
  m_queue(queue),
  m_client(NULL),
  m_helpDialog(NULL)
{
  ui->setupUi(this);

  reset();

  connect(ui->edit_submissionCommand, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->edit_killCommand, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->edit_requestQueueCommand, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->updateIntervalSpin, SIGNAL(valueChanged(int)),
          this, SLOT(setDirty()));
  connect(ui->edit_launchScriptName, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->edit_workingDirectoryBase, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->sshExecutableEdit, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->scpExecutableEdit, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->edit_hostName, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->edit_userName, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->spin_sshPort, SIGNAL(valueChanged(int)),
          this, SLOT(setDirty()));
  connect(ui->text_launchTemplate, SIGNAL(textChanged()),
          this, SLOT(setDirty()));

  connect(ui->push_testConnection, SIGNAL(clicked()),
          this, SLOT(testConnection()));
  connect(ui->push_sleepTest, SIGNAL(clicked()),
          this, SLOT(sleepTest()));
  connect(ui->templateHelpButton, SIGNAL(clicked()),
          this, SLOT(showTemplateHelp()));
}

RemoteQueueWidget::~RemoteQueueWidget()
{
  delete ui;
}

void RemoteQueueWidget::save()
{
  m_queue->setSubmissionCommand(ui->edit_submissionCommand->text());
  m_queue->setKillCommand(ui->edit_killCommand->text());
  m_queue->setRequestQueueCommand(ui->edit_requestQueueCommand->text());
  m_queue->setLaunchScriptName(ui->edit_launchScriptName->text());
  m_queue->setWorkingDirectoryBase(ui->edit_workingDirectoryBase->text());
  m_queue->setSshExecutable(ui->sshExecutableEdit->text());
  m_queue->setScpExecutable(ui->scpExecutableEdit->text());
  m_queue->setHostName(ui->edit_hostName->text());
  m_queue->setUserName(ui->edit_userName->text());
  m_queue->setSshPort(ui->spin_sshPort->value());

  m_queue->setQueueUpdateInterval(ui->updateIntervalSpin->value());

  QString text = ui->text_launchTemplate->document()->toPlainText();
  m_queue->setLaunchTemplate(text);

  int hours = ui->wallTimeHours->value();
  int minutes = ui->wallTimeMinutes->value();
  m_queue->setDefaultMaxWallTime(minutes + (hours * 60));
  setDirty(false);
}

void RemoteQueueWidget::reset()
{
  ui->edit_submissionCommand->setText(m_queue->submissionCommand());
  ui->edit_killCommand->setText(m_queue->killCommand());
  ui->edit_requestQueueCommand->setText(m_queue->requestQueueCommand());
  ui->edit_launchScriptName->setText(m_queue->launchScriptName());
  ui->edit_workingDirectoryBase->setText(m_queue->workingDirectoryBase());
  ui->updateIntervalSpin->setValue(m_queue->queueUpdateInterval());
  int walltime = m_queue->defaultMaxWallTime();
  ui->wallTimeHours->setValue(walltime / 60);
  ui->wallTimeMinutes->setValue(walltime % 60);
  ui->sshExecutableEdit->setText(m_queue->sshExecutable());
  ui->scpExecutableEdit->setText(m_queue->scpExectuable());
  ui->edit_hostName->setText(m_queue->hostName());
  ui->edit_userName->setText(m_queue->userName());
  ui->spin_sshPort->setValue(m_queue->sshPort());
  ui->text_launchTemplate->document()->setPlainText(m_queue->launchTemplate());
  setDirty(false);
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
  else if (m_queue->killCommand().isEmpty())
    missingVariable = tr("job cancel command");
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
    m_client = new LocalSocketClient (this);
    m_client->connectToServer();
  }

  JobRequest sleepJob = m_client->newJobRequest();
  sleepJob.setQueue(m_queue->name());
  sleepJob.setProgram(sleepProgram->name());
  sleepJob.setDescription("sleep 30 (test)");

  m_client->submitJobRequest(sleepJob);
}

void RemoteQueueWidget::showHelpDialog()
{
  if (!m_helpDialog)
    m_helpDialog = new TemplateKeywordDialog(this);
  m_helpDialog->show();
}

} // end namespace MoleQueue
