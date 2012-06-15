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

#include <QtGui/QMainWindow>

#include "molequeueglobal.h"
#include "connectionlistener.h"

#include <QtGui/QSystemTrayIcon>

#include <QtNetwork/QAbstractSocket>

class QAction;
class QIcon;

namespace Ui {
class MainWindow;
}

namespace MoleQueue
{
class Job;
class JobItemModel;
class Server;

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

protected slots:
  void showQueueManager();
  void handleServerConnectionError(ConnectionListener::Error,
                                   const QString &);

  void notifyJobStateChanged(const MoleQueue::Job *job,
                             MoleQueue::JobState oldState,
                             MoleQueue::JobState newState);

protected:
  void closeEvent(QCloseEvent *theEvent);

  void createActions();
  void createMainMenu();
  void createTrayIcon();
  void createJobTable();

  /** Our MainWindow GUI. */
  Ui::MainWindow *m_ui;

  QAction *m_minimizeAction;
  QAction *m_maximizeAction;
  QAction *m_restoreAction;

  QIcon *m_icon;
  QSystemTrayIcon *m_trayIcon;
  QMenu *m_trayIconMenu;

  Server *m_server;
};

} // End namespace

#endif
