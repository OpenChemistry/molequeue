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

#ifndef PROGRAMCONFIGUREDIALOG_H
#define PROGRAMCONFIGUREDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
class ProgramConfigureDialog;
}

namespace MoleQueue
{
class Program;

class ProgramConfigureDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ProgramConfigureDialog(Program *program, QWidget *parentObject = 0);
  ~ProgramConfigureDialog();

  Program *currentProgram() const { return m_program; }

public slots:
  void accept();
  // Disable the program name from being changed
  void lockName(bool locked);

protected slots:
  void populateSyntaxCombo();

  void updateGuiFromProgram();
  void updateProgramFromGui();

  void updateLaunchEditor();
  void launchEditorTextChanged();

  void launchSyntaxChanged(int enumVal);
  void customizeLauncherClicked();

  void importProgramClicked();

private:
  Ui::ProgramConfigureDialog *ui;
  Program *m_program;
  bool m_isCustomized;
  QString m_customLaunchText;
};

} // end namespace MoleQueue

#endif // PROGRAMCONFIGUREDIALOG_H
