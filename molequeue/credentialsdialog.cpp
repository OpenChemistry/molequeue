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

#include "credentialsdialog.h"
#include "ui_credentialsdialog.h"
#include "qdebug.h"

namespace MoleQueue
{

CredentialsDialog::CredentialsDialog(QWidget *parentObject) :
  QDialog(parentObject),
  ui(new Ui::CredentialsDialog)
{
  ui->setupUi(this);

  connect(ui->buttonBox, SIGNAL(rejected()),
          this, SIGNAL(cancelled()));

}

CredentialsDialog::~CredentialsDialog()
{
  delete ui;
}

void CredentialsDialog::accept()
{
  emit entered(ui->credentialsEdit->text());
  ui->credentialsEdit->clear();
  ui->messageLabel->clear();
  QDialog::accept();
}

void CredentialsDialog::reject()
{
  ui->credentialsEdit->clear();
  ui->messageLabel->clear();
  QDialog::reject();
}

void CredentialsDialog::setHostString(const QString &hostString)
{
  ui->hostLabel->setText(hostString);
}

void CredentialsDialog::setPrompt(const QString &prompt)
{
  ui->promptLabel->setText(prompt);
}

void CredentialsDialog::setErrorMessage(const QString &message) {
  ui->messageLabel->setText(message);
}
} // end namespace MoleQueue
