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

#ifndef MOLEQUEUE_IMPORTQUEUEDIALOG_H
#define MOLEQUEUE_IMPORTQUEUEDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
class ImportQueueDialog;
}

namespace MoleQueue {
class QueueManager;

class ImportQueueDialog : public QDialog
{
  Q_OBJECT

public:
  ImportQueueDialog(QueueManager *queueManager,
                    QWidget *parentObject = 0);
  ~ImportQueueDialog();

public slots:
  void accept();

private slots:
  void showImportFileDialog();
  void importFileTextChanged(const QString &text);

private:
  Ui::ImportQueueDialog *ui;
  QueueManager *m_queueManager;
};

} // namespace MoleQueue
#endif // MOLEQUEUE_IMPORTQUEUEDIALOG_H
