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

#include "filebrowsewidget.h"
#include "program.h"
#include "queue.h"
#include "queues/local.h"
#include "templatekeyworddialog.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtGui/QRegExpValidator>
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
  m_helpDialog(NULL),
  m_isCustomized((m_program->launchSyntax() == Program::CUSTOM)),
  m_dirty(true),
  m_isLocal((m_program != NULL
    && qobject_cast<QueueLocal*>(m_program->queue()) != NULL))
{
  ui->setupUi(this);

  setExecutableWidget();

  populateSyntaxCombo();

  connect(ui->combo_syntax, SIGNAL(currentIndexChanged(int)),
          this, SLOT(launchSyntaxChanged(int)));
  connect(ui->push_customize, SIGNAL(clicked()),
          this, SLOT(customizeLauncherClicked()));
  connect(ui->edit_arguments, SIGNAL(textChanged(QString)),
          this, SLOT(updateLaunchEditor()));
  connect(ui->edit_outputFilename, SIGNAL(textChanged(QString)),
          this, SLOT(updateLaunchEditor()));
  connect(ui->text_launchTemplate, SIGNAL(textChanged()),
          this, SLOT(launchEditorTextChanged()));
  connect(ui->templateHelpButton, SIGNAL(clicked()),
          this, SLOT(showHelpDialog()));

  connect(ui->edit_name, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->edit_arguments, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->edit_outputFilename, SIGNAL(textChanged(QString)),
          this, SLOT(setDirty()));
  connect(ui->combo_syntax, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setDirty()));
  connect(ui->push_customize, SIGNAL(clicked()),
          this, SLOT(setDirty()));
  connect(ui->text_launchTemplate, SIGNAL(textChanged()),
          this, SLOT(setDirty()));

  connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
          this, SLOT(buttonBoxButtonClicked(QAbstractButton*)));

  updateGuiFromProgram();

  launchSyntaxChanged(ui->combo_syntax->currentIndex());

  ui->edit_name->setValidator(new QRegExpValidator(
                                QRegExp(VALID_NAME_REG_EXP)));

  setDirty(false);
}

ProgramConfigureDialog::~ProgramConfigureDialog()
{
  delete ui;
}

void ProgramConfigureDialog::accept()
{
  if (m_dirty)
    if (!updateProgramFromGui())
      return;

  QDialog::accept();
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
      syntaxList << tr("Input as argument, redirect output");
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
  setExecutableName(m_program->executable());
  ui->edit_arguments->setText(m_program->arguments());
  ui->edit_outputFilename->setText(m_program->outputFilename());


  Program::LaunchSyntax syntax = m_program->launchSyntax();
  ui->combo_syntax->blockSignals(true);
  ui->combo_syntax->setCurrentIndex(static_cast<int>(syntax));
  ui->combo_syntax->blockSignals(false);
  m_customLaunchText = m_program->customLaunchTemplate();

  updateLaunchEditor();
  setDirty(false);
}

bool ProgramConfigureDialog::updateProgramFromGui()
{
  // If the name changed, check that it won't collide with an existing program.
  QString name = ui->edit_name->text().trimmed();
  if (name != m_program->name()) {
    if (Queue *queue = m_program->queue()) {
      if (queue->programNames().contains(name)) {
        int reply =
            QMessageBox::warning(this, tr("Name conflict"),
                                 tr("The program name has been changed to '%1',"
                                    " but there is already a program with that "
                                    "name.\n\nOverwrite existing program?")
                                 .arg(name), QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No);
        if (reply != QMessageBox::Yes) {
          ui->edit_name->selectAll();
          ui->edit_name->setFocus();
          return false;
        }
      }
      m_program->setName(name);
    }
  }

  m_program->setExecutable(executableName());
  m_program->setArguments(ui->edit_arguments->text());
  m_program->setOutputFilename(ui->edit_outputFilename->text());

  Program::LaunchSyntax syntax = static_cast<Program::LaunchSyntax>(
        ui->combo_syntax->currentIndex());
  m_program->setLaunchSyntax(syntax);
  m_program->setCustomLaunchTemplate(m_customLaunchText);
  setDirty(false);

  return true;
}

void ProgramConfigureDialog::updateLaunchEditor()
{
  Program::LaunchSyntax syntax = static_cast<Program::LaunchSyntax>(
        ui->combo_syntax->currentIndex());

  if (syntax == Program::CUSTOM) {
    ui->text_launchTemplate->document()->setPlainText(m_customLaunchText);
    return;
  }

  QString launchText;
  if (!m_program->queue() ||
      (qobject_cast<QueueLocal*>(m_program->queue()) &&
       syntax != Program::CUSTOM)) {
    launchText = "$$programExecution$$\n";
  }
  else {
    launchText = m_program->queue()->launchTemplate();
  }

  const QString executable     = executableName();
  const QString arguments      = ui->edit_arguments->text();
  const QString outputFilename = ui->edit_outputFilename->text();

  QString programExecution = Program::generateFormattedExecutionString(
        executable, arguments, outputFilename, syntax);

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
  Program::LaunchSyntax syntax = static_cast<Program::LaunchSyntax>(
        ui->combo_syntax->currentIndex());

  if (qobject_cast<QueueLocal*>(m_program->queue()) &&
      syntax != Program::CUSTOM) {
    m_customLaunchText = m_program->queue()->launchTemplate();
    QString execStr = ui->text_launchTemplate->document()->toPlainText();
    m_customLaunchText.replace("$$programExecution$$", execStr);
  }
  else {
    m_customLaunchText = ui->text_launchTemplate->document()->toPlainText();
  }

  ui->combo_syntax->setCurrentIndex(static_cast<int>(Program::CUSTOM));
}

void ProgramConfigureDialog::setDirty(bool dirty)
{
  if (dirty != m_dirty) {
    m_dirty = dirty;
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(m_dirty);
  }
}

void ProgramConfigureDialog::closeEvent(QCloseEvent *e)
{
  if (m_dirty) {
    // apply or discard changes?
    QMessageBox::StandardButton reply =
        QMessageBox::warning(this, tr("Unsaved changes"),
                             tr("The changes to the program have not been "
                                "saved. Would you like to save or discard "
                                "them?"),
                             QMessageBox::Save | QMessageBox::Discard |
                             QMessageBox::Cancel,
                             QMessageBox::Save);

    switch (reply) {
    case QMessageBox::Cancel:
      e->ignore();
      return;
    case QMessageBox::Save:
      if (!updateProgramFromGui())
        return;
    case QMessageBox::NoButton:
    case QMessageBox::Discard:
    default:
      e->accept();
      break;
    }
  }

  QDialog::closeEvent(e);
}

void ProgramConfigureDialog::keyPressEvent(QKeyEvent *e)
{
  // By default, the escape key bypasses the close event, but we still want to
  // check if the settings widget is dirty.
  if (e->key() == Qt::Key_Escape) {
    e->accept();
    close();
    return;
  }

  QDialog::keyPressEvent(e);
}

void ProgramConfigureDialog::setExecutableWidget()
{
  // Allow local file browsing if the program is used with a local queue.
  if (m_isLocal) {
    ui->label_executable->setBuddy(ui->browse_localExecutable);
    ui->browse_localExecutable->setMode(FileBrowseWidget::ExecutableFile);
    connect(ui->browse_localExecutable, SIGNAL(fileNameChanged(QString)),
            SLOT(updateLaunchEditor()));
    connect(ui->browse_localExecutable, SIGNAL(fileNameChanged(QString)),
            SLOT(setDirty()));
  }
  else { // Just use a line edit if the queue is remote.
    ui->label_executable->setBuddy(ui->edit_remoteExecutable);
    connect(ui->edit_remoteExecutable, SIGNAL(textChanged(QString)),
            SLOT(updateLaunchEditor()));
    connect(ui->edit_remoteExecutable, SIGNAL(textChanged(QString)),
            SLOT(setDirty()));
  }

  ui->browse_localExecutable->setVisible(m_isLocal);
  ui->edit_remoteExecutable->setVisible(!m_isLocal);
}

QString ProgramConfigureDialog::executableName() const
{
  return m_isLocal ? ui->browse_localExecutable->fileName()
                   : ui->edit_remoteExecutable->text();
}

void ProgramConfigureDialog::setExecutableName(const QString &name)
{
  if (m_isLocal)
    ui->browse_localExecutable->setFileName(name);
  else
    ui->edit_remoteExecutable->setText(name);
}

void ProgramConfigureDialog::showHelpDialog()
{
  if (!m_helpDialog)
    m_helpDialog = new TemplateKeywordDialog(this);
  m_helpDialog->show();
}

void ProgramConfigureDialog::buttonBoxButtonClicked(QAbstractButton *button)
{
  // "Ok" and "Cancel" are directly connected to accept() and reject(), so only
  // check for "apply" here:
  if (button == ui->buttonBox->button(QDialogButtonBox::Apply))
    updateProgramFromGui();
}

} // end namespace MoleQueue
