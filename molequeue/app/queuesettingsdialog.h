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

#ifndef MOLEQUEUE_QUEUESETTINGSDIALOG_H
#define MOLEQUEUE_QUEUESETTINGSDIALOG_H

#include <QtWidgets/QDialog>

#include <QtCore/QMap>

class QAbstractButton;
class QItemSelection;
class QModelIndex;

namespace Ui {
    class QueueSettingsDialog;
}

namespace MoleQueue
{
class AbstractQueueSettingsWidget;
class Program;
class ProgramConfigureDialog;
class Queue;
class QueueProgramItemModel;

/// @brief Dialog for configuring queues and managing programs.
class QueueSettingsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QueueSettingsDialog(Queue *queue, QWidget *parentObject = 0);
  ~QueueSettingsDialog();

  Queue *currentQueue() const { return m_queue; }

public slots:
  void accept();

protected slots:
  void addProgramClicked();
  void removeProgramClicked();
  void configureProgramClicked();
  void importProgramClicked();
  void exportProgramClicked();
  void doubleClicked(const QModelIndex &);
  void enableProgramButtons(const QItemSelection &selected);
  void showProgramConfigDialog(Program *prog);
  void setEnabledProgramButtons(bool enabled);
  void removeProgramDialog();
  void buttonBoxButtonClicked(QAbstractButton*);
  bool apply();
  void reset();
  void setDirty(bool dirty = true);
  void tabChanged(int index);

protected:
  void closeEvent(QCloseEvent *);
  void keyPressEvent(QKeyEvent *);

  /// Row indices, ascending order
  QList<int> getSelectedRows();
  QList<Program*> getSelectedPrograms();

  Ui::QueueSettingsDialog *ui;
  Queue *m_queue;
  QueueProgramItemModel *m_model;
  QMap<Program *, ProgramConfigureDialog *> m_programConfigureDialogs;
  AbstractQueueSettingsWidget *m_settingsWidget;
  bool m_dirty;
};

} // end MoleQueue namespace

#endif // QUEUESETTINGSDIALOG_H
