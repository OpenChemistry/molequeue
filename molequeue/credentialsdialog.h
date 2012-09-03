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

#ifndef CREDENTIALSDIALOG_H
#define CREDENTIALSDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
class CredentialsDialog;
}

namespace MoleQueue
{

/**
 * @class CredentialsWidget credentialswidget.h <molequeue/credentialswidget.h>
 *
 * @brief A dialog for prompting user for security credentials.
 */
class CredentialsDialog: public QDialog
{
  Q_OBJECT

public:
  explicit CredentialsDialog(QWidget *parentObject = 0);
  ~CredentialsDialog();

  void setHostString(const QString &hostString);
  void setPrompt(const QString &prompt);
  void setErrorMessage(const QString &message);

public slots:
  void accept();
  void reject();

signals:
  void entered(const QString &credentials);

private:
  Ui::CredentialsDialog *ui;

};

} // end namespace MoleQueue

#endif //CREDENTIALSDIALOG_H
