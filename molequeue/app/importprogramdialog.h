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

#ifndef MOLEQUEUE_IMPORTPROGRAMDIALOG_H
#define MOLEQUEUE_IMPORTPROGRAMDIALOG_H

#include <QDialog>

namespace Ui {
class ImportProgramDialog;
}

namespace MoleQueue {
class Queue;

/// @brief Dialog for importing a program configuration from a file.
class ImportProgramDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ImportProgramDialog(Queue *queue, QWidget *parentObject = 0);
  ~ImportProgramDialog();

public slots:
  void accept();

private slots:
  void showImportFileDialog();
  void importFileTextChanged(const QString &text);

private:
  Ui::ImportProgramDialog *ui;
  Queue *m_queue;
};


} // namespace MoleQueue
#endif // MOLEQUEUE_IMPORTPROGRAMDIALOG_H
