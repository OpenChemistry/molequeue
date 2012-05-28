/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

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

#include <QtGui/QSystemTrayIcon>

#include <QtNetwork/QLocalSocket>

class QAction;
class QIcon;

namespace Ui {
class MainWindow;
}

namespace MoleQueue {

class Connection;
class Job;
class JobItemModel;
class Program;
class Queue;
class QueueManager;
class Server;
class ServerConnection;

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
  void handleServerError(QAbstractSocket::SocketError, const QString &);
  void newConnection(ServerConnection *conn);

  // ServerConnection handlers
  void queueListRequested();
  void jobSubmissionRequested(const Job *req);
  void jobCancellationRequested(IdType moleQueueId);

protected:
  void closeEvent(QCloseEvent *theEvent);

  void createActions();
  void createMainMenu();
  void createTrayIcon();
  void createJobModel();

  /** Our MainWindow GUI. */
  Ui::MainWindow *m_ui;

  QAction *m_minimizeAction;
  QAction *m_maximizeAction;
  QAction *m_restoreAction;

  QIcon *m_icon;
  QSystemTrayIcon *m_trayIcon;
  QMenu *m_trayIconMenu;

  Server *m_server;
  JobItemModel *m_jobItemModel;

  QueueManager *m_queueManager;

  QString m_tmpDir;
  QString m_localDir;
};

} // End namespace

#endif
