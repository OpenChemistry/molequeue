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
#include "QueueSGE.h"
#include "queuemanager.h"
#include "queuemanagerdialog.h"

#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QTimer>
#include <QtCore/QSettings>
#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

namespace MoleQueue {

MainWindow::MainWindow()
  : m_ui(new Ui::MainWindow),
    m_removeServer(false),
    m_connection(0)
{
  m_ui->setupUi(this);

  m_queueManager = new QueueManager(this);

  createActions();
  createMainMenu();
  createTrayIcon();
  createQueues();
  createJobModel();

  readSettings();

  m_trayIcon->show();

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
  writeSettings();

  delete m_ui;
}

void MainWindow::setVisible(bool visible)
{
  m_ui->actionMinimize->setEnabled(visible);
  m_ui->actionMaximize->setEnabled(!isMaximized());
  m_ui->actionRestore->setEnabled(isMaximized() || !visible);
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

void MainWindow::readSettings()
{
  QSettings settings;
  m_tmpDir = settings.value("tmpDir", QDir::tempPath() + "/MoleQueue").toString();
  m_localDir = settings.value("localDir",
                              QDir::homePath() + "/.molequeue/local").toString();

  // Process the queues.
  settings.beginGroup("Queues");
  foreach(Queue *queue, m_queueManager->queues()) {
    settings.beginGroup(queue->name());
    queue->readSettings(settings);
    settings.endGroup();
  }
  settings.endGroup();
}

void MainWindow::writeSettings()
{
  QSettings settings;
  settings.setValue("tmpDir"  , m_tmpDir);
  settings.setValue("localDir", m_localDir);

  // Process the queues.
  settings.beginGroup("Queues");
  foreach(Queue *queue, m_queueManager->queues()) {
    settings.beginGroup(queue->name());
    queue->writeSettings(settings);
    settings.endGroup();
  }
  settings.endGroup();
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
  connect(m_connection, SIGNAL(jobSubmitted(QString,QString,QString,QString)),
          this, SLOT(submitJob(QString,QString,QString,QString)));
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

void MainWindow::submitJob(const QString &queue, const QString &program,
                           const QString &title, const QString &input)
{
  Queue *q = 0;
  if (queue == "local")
    q = m_queueManager->queues()[0];
  else if (queue == "remote")
    q = m_queueManager->queues()[1];
  Program job = q->program(program);
  job.setTitle(title);
  QString inputFile = title;
  inputFile.replace(" ", "_");
  job.setInputFile(inputFile + ".inp");
  job.setInput(input);
  q->submit(job);
  qDebug() << "Mainwindow submitting job" << queue << program << title
           << inputFile << "\n\n" << input;
}

void MainWindow::showQueueManager()
{
  QueueManagerDialog dialog(m_queueManager, this);
  dialog.exec();
}

void MainWindow::moveFile()
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
  connect(m_ui->actionMinimize, SIGNAL(triggered()), this, SLOT(hide()));
  connect(m_ui->actionMaximize, SIGNAL(triggered()), this, SLOT(showMaximized()));
  connect(m_ui->actionRestore, SIGNAL(triggered()), this, SLOT(showNormal()));
}

void MainWindow::createMainMenu()
{
  connect(m_ui->actionTest, SIGNAL(triggered()), this, SLOT(showMessage()));
  connect(m_ui->actionMove, SIGNAL(triggered()), this, SLOT(moveFile()));
  connect(m_ui->actionQueueManager, SIGNAL(triggered()), this, SLOT(showQueueManager()));
  connect(m_ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void MainWindow::createTrayIcon()
{
  m_trayIconMenu = new QMenu(this);
  m_trayIconMenu->addAction(m_ui->actionMinimize);
  m_trayIconMenu->addAction(m_ui->actionMaximize);
  m_trayIconMenu->addAction(m_ui->actionRestore);
  m_trayIconMenu->addSeparator();
  m_trayIconMenu->addAction(m_ui->actionQuit);

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
}

void MainWindow::createJobModel()
{
  m_jobModel = new ProgramItemModel(this);

  m_queueManager->addQueue(new QueueLocal(this));
  m_jobModel->addQueue(m_queueManager->queues().back());

  m_ui->jobView->setModel(m_jobModel);
  m_ui->jobView->setAlternatingRowColors(true);
  m_ui->jobView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_ui->jobView->setRootIsDecorated(false);
  m_ui->jobView->header()->setStretchLastSection(false);
  m_ui->jobView->header()->setResizeMode(0, QHeaderView::Stretch);
  //m_ui->jobView->header()->setResizeMode(0, QHeaderView::Stretch);

  Queue *queue = m_queueManager->queues().back();
  Program newJob = queue->program("sleep");
  newJob.setTitle("Test job...");
  newJob.setReplacement("time", "5");
//  queue->submit(newJob);

  newJob.setTitle("Test job longer...");
  newJob.setReplacement("time", "8");
//  queue->submit(newJob);
  newJob.setTitle("Test job longest...");
  newJob.setReplacement("time", "12");
//  queue->submit(newJob);

  // Now set up the remote queue
  m_queueManager->addQueue(new QueueSGE(this));
  queue = m_queueManager->queues().back();
  m_jobModel->addQueue(queue);
  Program remJob = queue->program("GAMESS");
  remJob.setTitle("benzene-gms");
  remJob.setReplacement("time", "5");
  remJob.setInputFile("benzene.inp");
  //queue->submit(remJob);
}

} // End namespace
