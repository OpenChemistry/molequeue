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

#include "importqueuedialog.h"
#include "ui_importqueuedialog.h"

#include "queue.h"
#include "queuemanager.h"

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QPalette>
#include <QtGui/QRegExpValidator>

#include <QtCore/QSettings>

namespace MoleQueue {

ImportQueueDialog::ImportQueueDialog(QueueManager *queueManager, QWidget *parentObject) :
  QDialog(parentObject),
  ui(new Ui::ImportQueueDialog),
  m_queueManager(queueManager)
{
  ui->setupUi(this);

  connect(ui->fileButton, SIGNAL(clicked()),
          this, SLOT(showImportFileDialog()));
  connect(ui->fileEdit, SIGNAL(textChanged(QString)),
          this, SLOT(importFileTextChanged(QString)));

  // Restrict queue names to alphanumeric strings with no leading whitespace.
  QRegExp queueRegExp("[0-9A-za-z][0-9A-Za-z ]*");
  ui->nameEdit->setValidator(new QRegExpValidator(queueRegExp));
}

ImportQueueDialog::~ImportQueueDialog()
{
  delete ui;
}

void ImportQueueDialog::accept()
{
  const QString name = ui->nameEdit->text();

  if (name.isEmpty()) {
    QMessageBox::critical(this, tr("Missing name"),
                          tr("Please enter a name for the queue before "
                             "continuing."), QMessageBox::Ok);
    return;
  }

  QSettings importer(ui->fileEdit->text(), QSettings::IniFormat);
  if (!importer.contains("type")) {
    QMessageBox::critical(this, tr("Cannot import queue!"),
                          tr("Cannot import queue from file '%1': File open "
                             "failed or invalid format.")
                          .arg(ui->fileEdit->text()),
                          QMessageBox::Ok);
    return;
  }

  QString type = importer.value("type").toString();
  if (!QueueManager::availableQueues().contains(type)) {
    QMessageBox::critical(this, tr("Cannot import queue!"),
                          tr("Cannot import queue from file '%1': Queue type "
                             "not recognized (%2).")
                          .arg(ui->fileEdit->text()).arg(type),
                          QMessageBox::Ok);
    return;
  }

  Queue *queue = m_queueManager->addQueue(name, type);

  if (queue) {
    queue->importConfiguration(importer, ui->importPrograms->isChecked());
    QDialog::accept();
    return;
  }

  // Queue could not be added. Inform user:
  QMessageBox::critical(this, tr("Cannot add queue"),
                        tr("Cannot add queue with queue name '%1', as an "
                           "existing queue already has this name. Please rename"
                           " it and try again.").arg(name));
}

void ImportQueueDialog::showImportFileDialog()
{
  // Get initial dir:
  QSettings settings;
  QString initialDir = settings.value("import/queue/lastImportFile",
                                      ui->fileEdit->text()).toString();
  if (initialDir.isEmpty())
    initialDir = QDir::homePath();

  initialDir = QFileInfo(initialDir).dir().absolutePath() +
      QString("/%1.mqq").arg(ui->nameEdit->text());

  // Get filename
  QString importFileName =
      QFileDialog::getOpenFileName(this, tr("Select file to import"),
                                   initialDir,
                                   tr("MoleQueue Queue Export Format (*.mqq);;"
                                      "All files (*)"));

  // User cancel:
  if (importFileName.isNull())
    return;

  // Set location for next time
  settings.setValue("import/queue/lastImportFile", importFileName);

  ui->fileEdit->setText(importFileName);
}

void ImportQueueDialog::importFileTextChanged(const QString &text)
{
  QPalette pal;
  pal.setColor(QPalette::Text,
               QFile::exists(text) ? Qt::darkGreen : Qt::red),
      ui->fileEdit->setPalette(pal);
}

} // namespace MoleQueue
