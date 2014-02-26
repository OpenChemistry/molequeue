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

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
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

  // Restrict queue names to alphanumeric strings with internal whitespace
  // (the input is trimmed() in accept()).
  ui->nameEdit->setValidator(new QRegExpValidator(
                               QRegExp(VALID_NAME_REG_EXP)));
}

ImportQueueDialog::~ImportQueueDialog()
{
  delete ui;
}

void ImportQueueDialog::accept()
{
  const QString name = ui->nameEdit->text().trimmed();

  if (name.isEmpty()) {
    QMessageBox::critical(this, tr("Missing name"),
                          tr("Please enter a name for the queue before "
                             "continuing."), QMessageBox::Ok);
    return;
  }

  QString type = Queue::queueTypeFromFile(ui->fileEdit->text());
  if (type.isEmpty()) {
    QMessageBox::critical(this, tr("Cannot import queue!"),
                          tr("Cannot import queue from file '%1': Cannot detect"
                             " queue type!")
                          .arg(ui->fileEdit->text()),
                          QMessageBox::Ok);
    return;
  }

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
    if (queue->importSettings(ui->fileEdit->text(),
                              ui->importPrograms->isChecked())) {
      QDialog::accept();
      return;
    }
    else {
      QMessageBox::critical(this, tr("Cannot add queue"),
                            tr("Error importing queue from file '%1'. Check the"
                               " log for details.").arg(ui->fileEdit->text()),
                            QMessageBox::Ok);
      return;
    }
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
