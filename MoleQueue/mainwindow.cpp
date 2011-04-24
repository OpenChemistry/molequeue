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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "terminalprocess.h"
#include "sshcommand.h"
#include "ProgramItemModel.h"
#include "program.h"
#include "Connection.h"
#include "QueueLocal.h"
#include "QueueRemote.h"

#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QTimer>
#include <QtCore/QDataStream>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

namespace MoleQueue {

MainWindow::MainWindow() : m_removeServer(false), m_connection(0)
{
  m_ui = new Ui::MainWindow;
  m_ui->setupUi(this);

  createActions();
  createMainMenu();
  createTrayIcon();
  createQueues();
  createJobModel();

  m_trayIcon->show();

  setWindowTitle(tr("MoleQueue"));
  resize(600, 300);

  // Start up our local socket server
  m_server = new QLocalServer(this);
  if (!m_server->listen("MoleQueue")) {
    QMessageBox::critical(this, tr("MoleQueue Server"),
                                tr("Unable to start the server: %1.")
                                  .arg(m_server->errorString()));
    //m_server->removeServer("MoleQueue");
    m_server->close();
    m_removeServer = true;

    qDebug() << "Creating a client connection...";
    // Create a test connection to the other server.
    QLocalSocket *socket = new QLocalSocket(this);
    socket->connectToServer("MoleQueue");
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
    connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)),
            this, SLOT(socketError(QLocalSocket::LocalSocketError)));
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));

    QTimer::singleShot(1000, this, SLOT(removeServer()));
//    close();
    return;
  }
  else {
    qDebug() << "Connecting server new connection up..."
             << m_server->fullServerName();
    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
  }
}

MainWindow::~MainWindow()
{
  delete m_ui;
  m_ui = 0;
}

 void MainWindow::setVisible(bool visible)
 {
   m_minimizeAction->setEnabled(visible);
   m_maximizeAction->setEnabled(!isMaximized());
   m_restoreAction->setEnabled(isMaximized() || !visible);
   QMainWindow::setVisible(visible);
 }

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (m_trayIcon->isVisible()) {
    QMessageBox::information(this, tr("Systray"),
                             tr("The program will keep running in the "
                                "system tray. To terminate the program, "
                                "choose <b>Quit</b> in the context menu "
                                "of the system tray entry."));
    hide();
    event->ignore();
  }
}

void MainWindow::setIcon(int index)
{
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
}

void MainWindow::showMessage()
{
  m_trayIcon->showMessage("Info",
                          "System tray resident queue manager initialized.",
                          QSystemTrayIcon::MessageIcon(0), 5000);
}

void MainWindow::messageClicked()
{
  QMessageBox::information(0, tr("Systray"),
                           tr("Sorry, I already gave what help I could.\n"
                              "Maybe you should try asking a human?"));
  createMessageGroupBox();
}

void MainWindow::newConnection()
{
  m_trayIcon->showMessage("Info",
                          tr("Client connected to us!"),
                          QSystemTrayIcon::MessageIcon(0), 5000);

  QLocalSocket *clientSocket = m_server->nextPendingConnection();
  if (!clientSocket) {
    qDebug() << "Erorr, invalid socket.";
    return;
  }

  connect(clientSocket, SIGNAL(disconnected()),
          clientSocket, SLOT(deleteLater()));

  m_connection = new Connection(clientSocket, this);
/*
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_7);
  out << static_cast<quint16>(0);
  out << QString("Hello from the server...");

  QList<QString> list;
  list << "GAMESS" << "MOPAC";

  out << list;

  out.device()->seek(0);
  out << static_cast<quint16>(block.size() - sizeof(quint16));

  qDebug() << "size:" << block.size() << sizeof(quint16) << "message:" << block;

  clientSocket->write(block);
  clientSocket->flush();
  clientSocket->disconnectFromServer();

  // Experimental QProcess for ssh
  qDebug() << "Calling SSH...";
  TerminalProcess ssh;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QProcessEnvironment sshEnv;
  if (env.contains("DISPLAY"))
    sshEnv.insert("DISPLAY", env.value("DISPLAY"));
  if (env.contains("EDITOR"))
    sshEnv.insert("EDITOR", env.value("EDITOR"));
  if (env.contains("SSH_AUTH_SOCK"))
    sshEnv.insert("SSH_AUTH_SOCK", env.value("SSH_AUTH_SOCK"));
  sshEnv.insert("SSH_ASKPASS", "/usr/bin/pinentry-qt4");
  ssh.setProcessEnvironment(sshEnv);
  ssh.setProcessChannelMode(QProcess::MergedChannels);
  //ssh.start("env");
  ssh.start("ssh", QStringList() << "unobtanium");// << "ls");
  if (!ssh.waitForStarted()) {
    qDebug() << "Failed to start SSH...";
    return;
  }
  ssh.waitForReadyRead();
  ssh.write("ls ~/\n");
  ssh.waitForBytesWritten();
  ssh.waitForReadyRead();
  QByteArray res = ssh.readAll();
  qDebug() << "ls:" << res;
  ssh.write("env\nexit\n");
  ssh.closeWriteChannel();
  if (!ssh.waitForFinished()) {
    ssh.close();
    qDebug() << "Failed to exit.";
  }
  QByteArray result = ssh.readAll();
  qDebug() << "Output:" << result;
*/
}

void MainWindow::socketReadyRead()
{
  m_trayIcon->showMessage("Info",
                          tr("Client connected to us!"),
                          QSystemTrayIcon::MessageIcon(0), 5000);
  qDebug() << "Ready to read...";
  m_removeServer = false;
}

void MainWindow::socketError(QLocalSocket::LocalSocketError socketError)
{
  switch (socketError) {
  case QLocalSocket::ServerNotFoundError:
    QMessageBox::information(this, tr("MoleQueue Client"),
                             tr("The pipe was not found. Please check the "
                                "local pipe name."));
    break;
  case QLocalSocket::ConnectionRefusedError:
    QMessageBox::information(this, tr("MoleQueue Client"),
                             tr("The connection was refused by the server. "
                                "Make sure the MoleQueue server is running, "
                                "and check that the local pipe name "
                                "is correct."));
    break;
  case QLocalSocket::PeerClosedError:
    break;
  default:
    QMessageBox::information(this, tr("MoleQueue Client"),
                             tr("The following error occurred: ."));
                             //.arg(socket->errorString()));
  }

  qDebug() << "Hit the soccket error!";
}

void MainWindow::socketConnected()
{
  qDebug() << "Socket connected...";
  m_removeServer = false;
}

void MainWindow::removeServer()
{
  if (m_removeServer) {
    qDebug() << "Removing the server, as it looks like there was a timeout.";
    m_server->removeServer("MoleQueue");
  }
  else {
    qDebug() << "Server not removed, client received response.";
  }
}

void MainWindow::moveFile()
{
  qDebug() << "Calling SSH...";
  TerminalProcess ssh;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QProcessEnvironment sshEnv;
  if (env.contains("DISPLAY"))
    sshEnv.insert("DISPLAY", env.value("DISPLAY"));
  if (env.contains("EDITOR"))
    sshEnv.insert("EDITOR", env.value("EDITOR"));
  if (env.contains("SSH_AUTH_SOCK"))
    sshEnv.insert("SSH_AUTH_SOCK", env.value("SSH_AUTH_SOCK"));
  sshEnv.insert("SSH_ASKPASS", "/usr/bin/pinentry-qt4");
  ssh.setProcessEnvironment(sshEnv);
  ssh.setProcessChannelMode(QProcess::MergedChannels);
  //ssh.start("env");
  ssh.start("scp", QStringList() << "MoleQueue.cbp"
            << "unobtanium:");// << "ls");
  if (!ssh.waitForStarted()) {
    qDebug() << "Failed to start SSH...";
    return;
  }
  ssh.closeWriteChannel();
  if (!ssh.waitForFinished()) {
    ssh.close();
    qDebug() << "Failed to exit.";
  }
  QByteArray result = ssh.readAll();
  qDebug() << "scp output:" << result << "Return code:" << ssh.exitCode();

  SshCommand command(this);
  command.setHostName("unobtanium");
  QString out;
  int exitCode = -1;
  bool success = command.execute("ls -la", out, exitCode);

  success = command.copyTo("MoleQueue.cbp", "MoleQueue666.cbp");
  success = command.copyDirTo("bin", "MoleQueueBin");

  // Now try copying some files over here!
  success = command.copyFrom("test.py", "test.py");
  success = command.copyDirFrom("src/cml-reader", "cml-reader");
}

void MainWindow::createIconGroupBox()
{
}

void MainWindow::createMessageGroupBox()
{
  m_trayIcon->showMessage("Info",
                          "System tray resident queue manager initialized.",
                          QSystemTrayIcon::MessageIcon(0), 15000);
}

void MainWindow::createActions()
{
  m_minimizeAction = new QAction(tr("Mi&nimize"), this);
  connect(m_minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

  m_maximizeAction = new QAction(tr("Ma&ximize"), this);
  connect(m_maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

  m_restoreAction = new QAction(tr("&Restore"), this);
  connect(m_restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

  m_quitAction = new QAction(tr("&Quit"), this);
  connect(m_quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void MainWindow::createMainMenu()
{
  QMenu *file = new QMenu(tr("&File"), m_ui->menubar);
  QAction *test = new QAction(tr("&Test"), this);
  connect(test, SIGNAL(triggered()), this, SLOT(showMessage()));
  file->addAction(test);

  QAction *move = new QAction(tr("&Move"), this);
  connect(move, SIGNAL(triggered()), this, SLOT(moveFile()));
  file->addAction(move);

  file->addAction(m_quitAction);
  m_ui->menubar->addMenu(file);
}

void MainWindow::createTrayIcon()
{
  m_trayIconMenu = new QMenu(this);
  m_trayIconMenu->addAction(m_minimizeAction);
  m_trayIconMenu->addAction(m_maximizeAction);
  m_trayIconMenu->addAction(m_restoreAction);
  m_trayIconMenu->addSeparator();
  m_trayIconMenu->addAction(m_quitAction);

  m_trayIcon = new QSystemTrayIcon(this);
  m_trayIcon->setContextMenu(m_trayIconMenu);

  m_icon = new QIcon(":/icons/avogadro.png");
  m_trayIcon->setIcon(*m_icon);

  if (m_trayIcon->supportsMessages())
    m_trayIcon->setToolTip("Queue manager...");
  else
    m_trayIcon->setToolTip("Queue manager (no message support)...");
}

void MainWindow::createQueues()
{
  m_queues.push_back(new QueueLocal);
  m_queues.push_back(new QueueRemote);

  Queue *queue = m_queues[0];
  Program gamess = queue->program("GAMESS");
  gamess.setInputFile("/home/marcus/build/gamess/methane/methane.inp");
  queue->submit(gamess);
}

void MainWindow::createJobModel()
{
  m_jobModel = new ProgramItemModel(&m_jobs, this);
  m_ui->jobView->setModel(m_jobModel);
  m_ui->jobView->setAlternatingRowColors(true);
  m_ui->jobView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_ui->jobView->setRootIsDecorated(false);
  m_ui->jobView->header()->setStretchLastSection(false);
  m_ui->jobView->header()->setResizeMode(0, QHeaderView::Stretch);
  //m_ui->jobView->header()->setResizeMode(0, QHeaderView::Stretch);

  Program *job = new Program;
  job->setName("MOPAC");
  job->setTitle("Test job...");
  m_jobModel->add(job);
  job = new Program;
  job->setName("GAMESS");
  job->setTitle("My GAMESS job...");
  m_jobModel->add(job);
}

} // End namespace
