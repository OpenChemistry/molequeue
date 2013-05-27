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

#ifndef OPENWITHMANAGERDIALOG_H
#define OPENWITHMANAGERDIALOG_H

#include <QtGui/QDialog>

#include <QtCore/QModelIndexList>

class QAbstractButton;
class QDataWidgetMapper;
class QItemDelegate;

namespace Ui {
class OpenWithManagerDialog;
}

namespace MoleQueue
{
class JobActionFactory;
class OpenWithActionFactory;
class OpenWithExecutableModel;
class OpenWithPatternModel;
class PatternTypeDelegate;

/// @brief Dialog window for configuring OpenWithActionFactory objects.
class OpenWithManagerDialog : public QDialog
{
  Q_OBJECT

public:
  explicit OpenWithManagerDialog(QWidget *parentObject = 0);
  ~OpenWithManagerDialog();

  void loadFactories();
  void reset();
  /** Return value is false if user refuses to accept changes. */
  bool apply();
  void accept();
  void reject();

protected:
  void closeEvent(QCloseEvent *);
  void keyPressEvent(QKeyEvent *);

private:
  /** Return values for validateExecutable() */
  enum ExecutableStatus {
    /** Executable is found and has correct permissions. */
    ExecOk = 0,
    /** Executable is not marked executable by the filesystem. */
    ExecNotExec,
    /** The executable cannot be found in the specified path. */
    ExecInvalidPath,
    /** The executable cannot be found in the system path. */
    ExecNotFound
  };

private slots:
  void buttonBoxClicked(QAbstractButton*);

  void markClean();
  void markDirty();

  void addFactory();
  void removeFactory();

  void factoryTypeChanged(int type);

  void browseExecutable();
  ExecutableStatus validateExecutable(const QString &executable);
  ExecutableStatus validateExecutable(const QString &executable,
                                      QString &executableFilePath);
  void testExecutable();
  void testExecutableMatch();
  void testExecutableNoMatch();
  void factorySelectionChanged();
  void setFactoryGuiEnabled(bool enable = true);

  void addPattern();
  void removePattern();
  void patternSelectionChanged();
  void patternDimensionsChanged();
  void setPatternGuiEnabled(bool enable = true);

  void checkTestText();
  void testTextMatch();
  void testTextNoMatch();

private:
  /**
   * @brief Search the environment variable PATH for a file with the specified
   * name.
   * @param exec The name of the file.
   * @return The absolute path to the file on the system, or a null QString if
   * not found.
   */
  static QString searchSystemPathForFile(const QString &exec);
  QModelIndexList selectedFactoryIndices() const;
  QModelIndexList selectedPatternIndices() const;
  OpenWithActionFactory *selectedFactory();
  QRegExp *selectedRegExp();

  Ui::OpenWithManagerDialog *ui;

  QList<OpenWithActionFactory> m_factories;
  QList<OpenWithActionFactory*> m_origFactories;
  OpenWithExecutableModel *m_factoryModel;
  OpenWithPatternModel *m_patternModel;
  QDataWidgetMapper *m_patternMapper;
  QDataWidgetMapper *m_handlerMapper;

  PatternTypeDelegate *m_patternTypeDelegate;

  bool m_dirty;
};

} // end namespace MoleQueue

#endif // OPENWITHMANAGERDIALOG_H
