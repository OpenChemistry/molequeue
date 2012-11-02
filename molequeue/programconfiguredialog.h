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

class QAbstractButton;

namespace Ui {
class ProgramConfigureDialog;
}

namespace MoleQueue
{
class Program;
class TemplateKeywordDialog;

/// @brief Dialog for setting Program configuration options.
class ProgramConfigureDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ProgramConfigureDialog(Program *program, QWidget *parentObject = 0);
  ~ProgramConfigureDialog();

  Program *currentProgram() const { return m_program; }

public slots:
  void accept();

protected slots:
  void populateSyntaxCombo();

  void updateGuiFromProgram();
  bool updateProgramFromGui();

  void updateLaunchEditor();
  void launchEditorTextChanged();

  void launchSyntaxChanged(int enumVal);
  void customizeLauncherClicked();

  void setDirty(bool dirty = true) { m_dirty = dirty; }

  void showHelpDialog();
  void buttonBoxButtonClicked(QAbstractButton*);

protected:
  void closeEvent(QCloseEvent *);
  void keyPressEvent(QKeyEvent *);

private:
  Ui::ProgramConfigureDialog *ui;
  Program *m_program;
  TemplateKeywordDialog *m_helpDialog;
  bool m_isCustomized;
  bool m_dirty;
  QString m_customLaunchText;
};

} // end namespace MoleQueue

#endif // PROGRAMCONFIGUREDIALOG_H
