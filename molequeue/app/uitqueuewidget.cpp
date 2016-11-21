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
#include "molequeueconfig.h"
#include "ui_uitqueuewidget.h"

#include "program.h"
#include "queues/queueuit.h"
#include "templatekeyworddialog.h"
#include <molequeue/client/client.h>
#include <molequeue/client/jobobject.h>

#include <QtCore/QTimer>
#include <QtCore/QSettings>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtGui/QTextDocument>
#include <QtWidgets/QFileDialog>

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
  connect(ui->editKerberosUserName, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->editKerberosRealm, SIGNAL(textChanged(QString)),
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
  connect(ui->pushGetHostNames, SIGNAL(clicked()),
          queue, SLOT(getUserHostAssoc()));
  connect(ui->hostNameComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setDirty()));
  connect(queue, SIGNAL(userHostAssocList(const Uit::UserHostAssocList&)),
          this, SLOT(updateHostList(const Uit::UserHostAssocList&)));
  connect(ui->text_launchTemplate, SIGNAL(textChanged()),
          this, SLOT(setDirty()));

}

UitQueueWidget::~UitQueueWidget()
{
  delete ui;
}

void UitQueueWidget::save()
{
  m_queue->setWorkingDirectoryBase(ui->editWorkingDirectoryBase->text());
  m_queue->setKerberosRealm(ui->editKerberosRealm->text());
  m_queue->setKerberosUserName(ui->editKerberosUserName->text());
  m_queue->setHostName(ui->hostNameComboBox->currentText());
  int index = ui->hostNameComboBox->currentIndex();
  m_queue->setHostID(ui->hostNameComboBox->itemData(index).toULongLong());
  m_queue->setQueueUpdateInterval(ui->updateIntervalSpin->value());

  QString text = ui->text_launchTemplate->document()->toPlainText();
  m_queue->setLaunchTemplate(text);

  int hours = ui->wallTimeHours->value();
  int minutes = ui->wallTimeMinutes->value();
  m_queue->setDefaultMaxWallTime(minutes + (hours * 60));
  setDirty(false);
}

void UitQueueWidget::reset()
{
  ui->editWorkingDirectoryBase->setText(m_queue->workingDirectoryBase());
  ui->updateIntervalSpin->setValue(m_queue->queueUpdateInterval());
  ui->editKerberosRealm->setText(m_queue->kerberosRealm());
  ui->editKerberosUserName->setText(m_queue->kerberosUserName());

  if (ui->hostNameComboBox->count() > 0) {
    int index = ui->hostNameComboBox->findText(m_queue->hostName());

    if (index == -1)
      index = 0;

    ui->hostNameComboBox->setCurrentIndex(index);
  }
  else {
    ui->hostNameComboBox->addItem(m_queue->hostName(), m_queue->hostId());
  }
  ui->text_launchTemplate->document()->setPlainText(m_queue->launchTemplate());
  setDirty(false);
}

void UitQueueWidget::testConnection()
{
  m_queue->testConnection(this);
}


void UitQueueWidget::sleepTest()
{
#ifdef MoleQueue_BUILD_CLIENT
  QString promptString;
  if (isDirty()) {
    promptString = tr("Would you like to apply the current settings and submit "
                      "a test job? The job will run 'sleep 30' on the remote "
                      "queue.");
  }
  else {
    promptString = tr("Would you like to submit a test job? The job will run "
                      "'sleep 30' on the remote queue.");
  }

  QMessageBox::StandardButton response =
      QMessageBox::question(this, tr("Submit test job?"), promptString,
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::Yes);

  if (response != QMessageBox::Yes)
    return;

  if (isDirty())
    save();

  // Check that important variables are set:
  QString missingVariable = "";
  if (m_queue->hostName().isEmpty())
    missingVariable = tr("server hostname");
  else if (m_queue->kerberosUserName().isEmpty())
    missingVariable = tr("kerberos username");
  else if (m_queue->kerberosRealm().isEmpty())
    missingVariable = tr("kerberos realm");
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
    sleepProgram->setOutputFilename("");
    sleepProgram->setLaunchSyntax(Program::PLAIN);
    m_queue->addProgram(sleepProgram);
  }

  if (!m_client) {
    m_client = new Client (this);
    m_client->connectToServer();
  }

  JobObject sleepJob;
  sleepJob.setQueue(m_queue->name());
  sleepJob.setProgram(sleepProgram->name());
  sleepJob.setDescription("sleep 30 (test)");

  m_client->submitJob(sleepJob);
#endif // MoleQueue_BUILD_CLIENT
}



void UitQueueWidget::showHelpDialog()
{
  if (!m_helpDialog)
    m_helpDialog = new TemplateKeywordDialog(this);
  m_helpDialog->show();
}

void UitQueueWidget::updateHostList(const Uit::UserHostAssocList &list)
{
  QList<Uit::UserHostAssoc> hostAssocs =  list.userHostAssocs();

  QString currentHost = ui->hostNameComboBox->currentText();
  ui->hostNameComboBox->clear();
  foreach(const Uit::UserHostAssoc hostAssoc, hostAssocs) {
    ui->hostNameComboBox->addItem(hostAssoc.hostName(), hostAssoc.hostId());
  }

  int index = ui->hostNameComboBox->findText(currentHost);
  if (index != -1)
    ui->hostNameComboBox->setCurrentIndex(index);
  else {
    ui->hostNameComboBox->insertItem(0, "Select Hostname ...");
    ui->hostNameComboBox->setCurrentIndex(0);
  }
}

} // end namespace MoleQueue
