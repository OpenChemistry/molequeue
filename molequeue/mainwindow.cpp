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

#include "job.h"
#include "terminalprocess.h"
#include "sshcommand.h"
#include "jobitemmodel.h"
#include "program.h"
#include "connection.h"
#include "queues/local.h"
#include "queues/remote.h"
#include "queues/sge.h"
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
    m_minimizeAction(NULL),
    m_maximizeAction(NULL),
    m_restoreAction(NULL),
    m_icon(NULL),
    m_trayIcon(NULL),
    m_trayIconMenu(NULL),
    m_server(NULL),
    m_removeServer(false),
    m_queueManager(NULL),
    m_jobModel(NULL),
    m_connection(NULL)
{
  m_ui->setupUi(this);

  m_queueManager = new QueueManager(this);

  createActions();
  createMainMenu();
  createTrayIcon();
  readSettings();
  createJobModel();

  // Start up our local socket server
  m_server = new QLocalServer(this);
  if (!m_server->listen("MoleQueue")) {
    // Already running -- display error message and exit.
    QMessageBox::critical(this, tr("MoleQueue Server"),
                                tr("Unable to start the server: %1.\n"
                                   "Exiting.")
                                  .arg(m_server->errorString()));
    m_server->close();
    m_removeServer = true;
    this->removeServer();
    QTimer::singleShot(0, qApp, SLOT(quit()));
    this->close();

    return;
  }
  else {
    qDebug() << "Connecting server new connection up..."
             << m_server->fullServerName();
    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
  }

  m_trayIcon->show();
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

void MainWindow::closeEvent(QCloseEvent *theEvent)
{
  if (m_trayIcon->isVisible()) {
    QMessageBox::information(this, tr("Systray"),
                             tr("The program will keep running in the "
                                "system tray. To terminate the program, "
                                "choose <b>Quit</b> in the context menu "
                                "of the system tray entry."));
    hide();
    theEvent->ignore();
  }
}

void MainWindow::readSettings()
{
  QSettings settings;
  m_tmpDir = settings.value(
        "tmpDir", QDir::tempPath() + "/MoleQueue").toString();
  m_localDir = settings.value(
        "localDir", QDir::homePath() + "/.molequeue/local").toString();

  // read names of queues
  QStringList queueNames = settings.value("queues").toStringList();

  // Process the queues.
  settings.beginGroup("Queues");
  foreach(const QString &queueName, queueNames){
    settings.beginGroup(queueName);
    QString type = settings.value("type").toString();

    Queue *queue = m_queueManager->createQueue(type);
    if(queue){
      queue->setName(queueName);
      queue->readSettings(settings);
      m_queueManager->addQueue(queue);
    }
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
  QStringList queueNames;
  settings.beginGroup("Queues");
  foreach(Queue *queue, m_queueManager->queues()) {
    settings.beginGroup(queue->name());
    settings.setValue("type", queue->typeName());
    queue->writeSettings(settings);
    settings.endGroup();
    queueNames.append(queue->name());
  }
  settings.endGroup();

  // write name of each queue
  settings.setValue("queues", queueNames);
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
  Job *job = q->program(program)->createJob();
  job->setTitle(title);
  QString inputFile = title;
  inputFile.replace(" ", "_");
  job->setInputFile(inputFile + ".inp");
  job->setInput(input);
  q->submit(job);
  qDebug() << "Mainwindow submitting job" << queue << program << title
           << inputFile << "\n\n" << input;
}

void MainWindow::showQueueManager()
{
  QueueManagerDialog dialog(m_queueManager, this);
  dialog.exec();
}

void MainWindow::createActions()
{
  connect(m_ui->actionMinimize, SIGNAL(triggered()), this, SLOT(hide()));
  connect(m_ui->actionMaximize, SIGNAL(triggered()), this, SLOT(showMaximized()));
  connect(m_ui->actionRestore, SIGNAL(triggered()), this, SLOT(showNormal()));
}

void MainWindow::createMainMenu()
{
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

void MainWindow::createJobModel()
{
  m_jobModel = new JobItemModel(this);

  foreach(Queue *queue, m_queueManager->queues()){
    m_jobModel->addQueue(queue);
  }

  m_ui->jobView->setModel(m_jobModel);
  m_ui->jobView->header()->setResizeMode(0, QHeaderView::Stretch);
}

} // End namespace
