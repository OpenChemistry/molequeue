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

#ifndef QUEUESETTINGSDIALOG_H
#define QUEUESETTINGSDIALOG_H

#include <QtGui/QDialog>

#include <QtCore/QMap>

class QItemSelection;
class QModelIndex;

namespace Ui {
    class QueueSettingsDialog;
}

namespace MoleQueue
{
class Program;
class ProgramConfigureDialog;
class Queue;
class QueueProgramItemModel;

class QueueSettingsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QueueSettingsDialog(Queue *queue, QWidget *parentObject = 0);
  ~QueueSettingsDialog();

  Queue *currentQueue() const { return m_queue; }

protected slots:
  void addProgramClicked();
  void removeProgramClicked();
  void configureProgramClicked();
  void exportProgramClicked();
  void doubleClicked(const QModelIndex &);
  void enableProgramButtons(const QItemSelection &selected);

protected:
  /// Row indices, ascending order
  QList<int> getSelectedRows();
  QList<Program*> getSelectedPrograms();
  void showProgramConfigDialog(Program *prog);
  void setEnabledProgramButtons(bool enabled);
  void removeProgramDialog();

  Ui::QueueSettingsDialog *ui;
  Queue *m_queue;
  QueueProgramItemModel *m_model;
  QMap<Program *, ProgramConfigureDialog *> m_programConfigureDialogs;
};

} // end MoleQueue namespace

#endif // QUEUESETTINGSDIALOG_H
