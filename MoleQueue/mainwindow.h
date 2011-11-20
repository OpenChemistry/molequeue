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
#include <QtGui/QSystemTrayIcon>
#include <QtNetwork/QLocalSocket>

class QAction;
class QIcon;
class QLocalServer;

namespace Ui {
class MainWindow;
}

namespace MoleQueue {

class Queue;
class Program;
class ProgramItemModel;
class Connection;
class QueueManager;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

  void setVisible(bool visible);

protected:
  void closeEvent(QCloseEvent *event);

public slots:
  void readSettings();
  void writeSettings();

private slots:
  /** Receive new job submissions, send them to the appropriate queue. */
  void submitJob(const QString &queue, const QString &program,
                 const QString &fileName, const QString &input);

  void setIcon(int index);
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void showMessage();
  void messageClicked();
  void newConnection();
  void socketReadyRead();
  void socketError(QLocalSocket::LocalSocketError socketError);
  void socketConnected();
  void removeServer();
  void showQueueManager();

  /** Move file to appropriate place for execution. */
  void moveFile();

private:
  void createMessageGroupBox();
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

  QLocalServer *m_server;

  bool m_removeServer;

  QueueManager *m_queueManager;

  QList<Program *> m_jobs;
  ProgramItemModel *m_jobModel;

  Connection *m_connection;

  QString m_tmpDir;
  QString m_localDir;
};

} // End namespace

#endif
