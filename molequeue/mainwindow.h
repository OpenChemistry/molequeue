/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "molequeueglobal.h"
#include "transport/connectionlistener.h"

#include <QtGui/QMainWindow>
#include <QtGui/QSystemTrayIcon>

#include <QtNetwork/QAbstractSocket>

class QAction;
class QIcon;
class QLabel;

namespace Ui {
class MainWindow;
}

namespace MoleQueue
{
class Job;
class JobItemModel;
class LogEntry;
class LogWindow;
class OpenWithManagerDialog;
class QueueManagerDialog;
class Server;

/// The main window for the MoleQueue application.
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

  void setVisible(bool visible);

public slots:
  void readSettings();
  void writeSettings();

  void trayIconActivated(QSystemTrayIcon::ActivationReason);

  void errorOccurred();
  void errorCleared();

  void notifyJobStateChange(const MoleQueue::Job &job,
                            MoleQueue::JobState oldState,
                            MoleQueue::JobState newState);

protected slots:
  void showQueueManagerDialog();
  void showOpenWithManagerDialog();
  void showLogWindow();
  void handleServerConnectionError(MoleQueue::ConnectionListener::Error,
                                   const QString &);
  void handleErrorNotificationLabelAction(const QString &action);
  void jumpToFilterBar();
  void updateJobCounts(int totalJobs, int shownJobs);

protected:
  void keyPressEvent(QKeyEvent *);
  void closeEvent(QCloseEvent *theEvent);

  void createActions();
  void createShortcuts();
  void createMainMenu();
  void createTrayIcon();
  void createJobTable();
  void createActionFactories();
  void createStatusBar();

  Ui::MainWindow *m_ui;
  LogWindow *m_logWindow;
  OpenWithManagerDialog *m_openWithManagerDialog;
  QueueManagerDialog *m_queueManagerDialog;

  QAction *m_minimizeAction;
  QAction *m_maximizeAction;
  QAction *m_restoreAction;

  QSystemTrayIcon *m_trayIcon;
  QMenu *m_trayIconMenu;
  QLabel *m_statusTotalJobs;
  QLabel *m_statusHiddenJobs;

  Server *m_server;
};

} // End namespace

#endif
