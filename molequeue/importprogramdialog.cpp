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

#include "importprogramdialog.h"
#include "ui_importprogramdialog.h"

#include "program.h"
#include "queue.h"

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QPalette>

#include <QtCore/QSettings>

namespace MoleQueue {

ImportProgramDialog::ImportProgramDialog(Queue *queue, QWidget *parentObject) :
  QDialog(parentObject),
  ui(new Ui::ImportProgramDialog),
  m_queue(queue)
{
  ui->setupUi(this);

  connect(ui->fileButton, SIGNAL(clicked()),
          this, SLOT(showImportFileDialog()));
  connect(ui->fileEdit, SIGNAL(textChanged(QString)),
          this, SLOT(importFileTextChanged(QString)));

}

ImportProgramDialog::~ImportProgramDialog()
{
  delete ui;
}

void ImportProgramDialog::accept()
{
  const QString name = ui->nameEdit->text();

  if (name.isEmpty()) {
    QMessageBox::critical(this, tr("Missing name"),
                          tr("Please enter a name for the program before "
                             "continuing."), QMessageBox::Ok);
    return;
  }

  QSettings importer(ui->fileEdit->text(), QSettings::IniFormat);
  if (!importer.contains("executable")) {
    QMessageBox::critical(this, tr("Cannot import program!"),
                          tr("Cannot import program from file '%1': File open "
                             "failed or invalid format.")
                          .arg(ui->fileEdit->text()),
                          QMessageBox::Ok);
    return;
  }

  Program *program = new Program(m_queue);
  program->setName(name);
  program->importConfiguration(importer);
  if (m_queue->addProgram(program, false)) {
    QDialog::accept();
    return;
  }

  // Program could not be added. Inform user:
  QMessageBox::critical(this, tr("Cannot add program"),
                        tr("Cannot add program with name '%1', as an "
                           "existing program already has this name. Please "
                           "rename it and try again.").arg(name));
  program->deleteLater();
}

void ImportProgramDialog::showImportFileDialog()
{
  // Get initial dir:
  QSettings settings;
  QString initialDir = settings.value("import/program/lastImportFile",
                                      ui->fileEdit->text()).toString();
  if (initialDir.isEmpty())
    initialDir = QDir::homePath();

  initialDir = QFileInfo(initialDir).dir().absolutePath() +
      QString("/%1-%2.mqp").arg(m_queue->name(), ui->nameEdit->text());

  // Get filename
  QString importFileName =
      QFileDialog::getOpenFileName(this, tr("Select file to import"),
                                   initialDir,
                                   tr("MoleQueue Program Export Format (*.mqp)"
                                      ";;All files (*)"));

  // User cancel:
  if (importFileName.isNull())
    return;

  // Set location for next time
  settings.setValue("import/program/lastImportFile", importFileName);

  ui->fileEdit->setText(importFileName);
}

void ImportProgramDialog::importFileTextChanged(const QString &text)
{
  QPalette pal;
  pal.setColor(QPalette::Text,
               QFile::exists(text) ? Qt::darkGreen : Qt::red),
      ui->fileEdit->setPalette(pal);
}

} // namespace MoleQueue
