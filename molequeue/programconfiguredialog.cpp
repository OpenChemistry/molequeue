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

#include "programconfiguredialog.h"
#include "ui_programconfiguredialog.h"

#include "program.h"
#include "queue.h"

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QTextDocument>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>

namespace MoleQueue
{

ProgramConfigureDialog::ProgramConfigureDialog(Program *program,
                                               QWidget *parentObject) :
  QDialog(parentObject),
  ui(new Ui::ProgramConfigureDialog),
  m_program(program),
  m_isCustomized((m_program->launchSyntax() == Program::CUSTOM))
{
  ui->setupUi(this);

  populateSyntaxCombo();

  connect(ui->combo_syntax, SIGNAL(currentIndexChanged(int)),
          this, SLOT(launchSyntaxChanged(int)));
  connect(ui->push_customize, SIGNAL(clicked()),
          this, SLOT(customizeLauncherClicked()));
  connect(ui->edit_executableName, SIGNAL(textChanged(QString)),
          this, SLOT(updateLaunchEditor()));
  connect(ui->edit_executablePath, SIGNAL(textChanged(QString)),
          this, SLOT(updateLaunchEditor()));
  connect(ui->edit_arguments, SIGNAL(textChanged(QString)),
          this, SLOT(updateLaunchEditor()));
  connect(ui->edit_inputFilename, SIGNAL(textChanged(QString)),
          this, SLOT(updateLaunchEditor()));
  connect(ui->edit_outputFilename, SIGNAL(textChanged(QString)),
          this, SLOT(updateLaunchEditor()));
  connect(ui->gb_executablePath, SIGNAL(toggled(bool)),
          this, SLOT(updateLaunchEditor()));
  connect(ui->text_launchTemplate, SIGNAL(textChanged()),
          this, SLOT(launchEditorTextChanged()));

  launchSyntaxChanged(ui->combo_syntax->currentIndex());

  updateGuiFromProgram();
}

ProgramConfigureDialog::~ProgramConfigureDialog()
{
  delete ui;
}

void ProgramConfigureDialog::accept()
{
  updateProgramFromGui();
  QDialog::accept();
}

void ProgramConfigureDialog::lockName(bool locked)
{
  ui->edit_name->setDisabled(locked);
}

void ProgramConfigureDialog::populateSyntaxCombo()
{
  QStringList syntaxList;
  for (int cur = 0; cur < static_cast<int>(Program::SYNTAX_COUNT); ++cur) {
    switch (static_cast<Program::LaunchSyntax>(cur)) {
    case Program::CUSTOM:
      syntaxList << tr("Custom");
      break;
    case Program::PLAIN:
      syntaxList << tr("Plain");
      break;
    case Program::INPUT_ARG:
      syntaxList << tr("Input as argument");
      break;
    case Program::INPUT_ARG_NO_EXT:
      syntaxList << tr("Input as argument (no extension)");
      break;
    case Program::REDIRECT:
      syntaxList << tr("Redirect input and output");
      break;
    case Program::INPUT_ARG_OUTPUT_REDIRECT:
      syntaxList << tr("Input as output, redirect output");
      break;
    case Program::SYNTAX_COUNT:
    default:
      continue;
    }
  }

  ui->combo_syntax->blockSignals(true);
  ui->combo_syntax->clear();
  ui->combo_syntax->addItems(syntaxList);
  ui->combo_syntax->blockSignals(false);
}

void ProgramConfigureDialog::updateGuiFromProgram()
{
  ui->edit_name->setText(m_program->name());
  ui->edit_executableName->setText(m_program->executable());
  ui->gb_executablePath->setChecked(m_program->useExecutablePath());
  ui->edit_executablePath->setText(m_program->executablePath());
  ui->edit_arguments->setText(m_program->arguments());
  ui->edit_inputFilename->setText(m_program->inputFilename());
  ui->edit_outputFilename->setText(m_program->outputFilename());


  Program::LaunchSyntax syntax = m_program->launchSyntax();
  ui->combo_syntax->blockSignals(true);
  ui->combo_syntax->setCurrentIndex(static_cast<int>(syntax));
  ui->combo_syntax->blockSignals(false);
  m_customLaunchText = m_program->customLaunchTemplate();

  updateLaunchEditor();
}

void ProgramConfigureDialog::updateProgramFromGui()
{
  m_program->setName(ui->edit_name->text());
  m_program->setExecutable(ui->edit_executableName->text());
  m_program->setUseExecutablePath(ui->gb_executablePath->isChecked());
  m_program->setExecutablePath(ui->edit_executablePath->text());
  m_program->setArguments(ui->edit_arguments->text());
  m_program->setInputFilename(ui->edit_inputFilename->text());
  m_program->setOutputFilename(ui->edit_outputFilename->text());

  Program::LaunchSyntax syntax = static_cast<Program::LaunchSyntax>(
        ui->combo_syntax->currentIndex());
  m_program->setLaunchSyntax(syntax);
  m_program->setCustomLaunchTemplate(m_customLaunchText);
}

void ProgramConfigureDialog::updateLaunchEditor()
{
  Program::LaunchSyntax syntax = static_cast<Program::LaunchSyntax>(
        ui->combo_syntax->currentIndex());

  if (syntax == Program::CUSTOM) {
    ui->text_launchTemplate->document()->setPlainText(m_customLaunchText);
    return;
  }

  QString launchText = m_program->queue() ? m_program->queue()->launchTemplate()
                                          : QString("$$programExecution$$\n");

  const QString executableName = ui->edit_executableName->text();
  const QString executablePath = ui->edit_executablePath->text();
  const QString arguments      = ui->edit_arguments->text();
  const QString inputFilename  = ui->edit_inputFilename->text();
  const QString outputFilename = ui->edit_outputFilename->text();
  const bool useExecutablePath = ui->gb_executablePath->isChecked();

  QString programExecution = Program::generateFormattedExecutionString(
        executableName, arguments, inputFilename, outputFilename,
        executablePath, useExecutablePath, syntax);

  launchText.replace("$$programExecution$$", programExecution);

  ui->text_launchTemplate->document()->setPlainText(launchText);
}

void ProgramConfigureDialog::launchEditorTextChanged()
{
  QString launchText = ui->text_launchTemplate->document()->toPlainText();
  Program::LaunchSyntax syntax = static_cast<Program::LaunchSyntax>(
        ui->combo_syntax->currentIndex());

  if (syntax == Program::CUSTOM)
    m_customLaunchText = launchText;

  /// @todo Syntax highlighting?

}

void ProgramConfigureDialog::launchSyntaxChanged(int enumVal)
{
  Q_UNUSED(enumVal);

  Program::LaunchSyntax syntax = static_cast<Program::LaunchSyntax>(enumVal);

  bool syntaxIsCustom = (syntax == Program::CUSTOM);

  ui->push_customize->setDisabled(syntaxIsCustom);
  ui->text_launchTemplate->setReadOnly(!syntaxIsCustom);

  updateLaunchEditor();
}

void ProgramConfigureDialog::customizeLauncherClicked()
{
  m_customLaunchText = ui->text_launchTemplate->document()->toPlainText();
  ui->combo_syntax->setCurrentIndex(static_cast<int>(Program::CUSTOM));
}

} // end namespace MoleQueue
