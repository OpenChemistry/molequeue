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

#include "connection.h"
#include "job.h"
#include "jobitemmodel.h"
#include "jobmanager.h"
#include "program.h"
#include "queuemanager.h"
#include "queuemanagerdialog.h"
#include "queues/local.h"
#include "queues/remote.h"
#include "queues/sge.h"
#include "server.h"
#include "serverconnection.h"
#include "sshcommand.h"
#include "terminalprocess.h"

#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QSettings>
#include <QtCore/QTimer>

#include <QtGui/QCloseEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>

namespace MoleQueue {

MainWindow::MainWindow()
  : m_ui(new Ui::MainWindow),
    m_minimizeAction(NULL),
    m_maximizeAction(NULL),
    m_restoreAction(NULL),
    m_icon(NULL),
    m_trayIcon(NULL),
    m_trayIconMenu(NULL),
    m_server(new Server (this)),
    m_jobItemModel(new JobItemModel (m_server->jobManager(), this)),
    m_queueManager(new QueueManager (this))
{
  m_ui->setupUi(this);

  createActions();
  createMainMenu();
  createTrayIcon();
  readSettings();
  createJobModel();

  connect(m_server, SIGNAL(connectionError(QAbstractSocket::SocketError,QString)),
          this, SLOT(handleServerError(QAbstractSocket::SocketError, QString)));

  // Must be a direct connection to ensure that connections are made before the
  // socket starts processing and announcing requests.
  connect(m_server, SIGNAL(newConnection(ServerConnection*)),
          this, SLOT(newConnection(ServerConnection*)), Qt::DirectConnection);

  m_server->setDebug(true);
  m_server->start();

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

void MainWindow::showQueueManager()
{
  QueueManagerDialog dialog(m_queueManager, this);
  dialog.exec();
}

void MainWindow::handleServerError(QAbstractSocket::SocketError err,
                                   const QString &str)
{
  // handle AddressInUseError by giving user option to replace current socket
  if (err == QAbstractSocket::AddressInUseError) {
    QStringList choices;
    choices << tr("There is no other server running. Continue running.")
            << tr("Oops -- there is an existing server. Terminate the new server.");
    bool ok;
    QString choice =
        QInputDialog::getItem(this, tr("Replace existing MoleQueue server?"),
                              tr("A MoleQueue server appears to already be "
                                 "running. How would you like to handle this?"),
                              choices, 0, false, &ok);
    int index = choices.indexOf(choice);

    // Terminate
    if (!ok || index == 1) {
      this->hide();
      exit(0);
    }
    // Take over connection
    else {
      m_server->forceStart();
    }
  }

  // Any other error -- just notify user:
  else {
    QMessageBox::warning(this, tr("Server error"),
                         tr("A server error has occurred: '%1'").arg(str));
  }
}

void MainWindow::newConnection(ServerConnection *conn)
{
  connect(conn, SIGNAL(queueListRequested()), this, SLOT(queueListRequested()));
  connect(conn, SIGNAL(jobSubmissionRequested(const Job*)),
          this, SLOT(jobSubmissionRequested(const Job*)));
  connect(conn, SIGNAL(jobCancellationRequested(IdType)),
          this, SLOT(jobCancellationRequested(IdType)));
}

void MainWindow::queueListRequested()
{
  ServerConnection *conn = qobject_cast<ServerConnection*>(this->sender());
  if (conn == NULL) {
    qWarning() << Q_FUNC_INFO << "called with a sender which is not a "
                  "ServerConnection.";
    return;
  }

  conn->sendQueueList(m_queueManager->toQueueList());
}

void MainWindow::jobSubmissionRequested(const Job *req)
{
  ServerConnection *conn = qobject_cast<ServerConnection*>(this->sender());
  if (conn == NULL) {
    qWarning() << Q_FUNC_INFO << "called with a sender which is not a "
                  "ServerConnection.";
    return;
  }

  qDebug() << "Job submission requested:\n" << req->hash();



  /// @todo Actually handle the submission

  conn->sendSuccessfulSubmissionResponse(req);
}

void MainWindow::jobCancellationRequested(IdType moleQueueId)
{
  ServerConnection *conn = qobject_cast<ServerConnection*>(this->sender());
  if (conn == NULL) {
    qWarning() << Q_FUNC_INFO << "called with a sender which is not a "
                  "ServerConnection.";
    return;
  }

  qDebug() << "Job cancellation requested: MoleQueueId:" << moleQueueId;

  const Job *req = m_server->jobManager()->lookupMoleQueueId(moleQueueId);

  /// @todo actually handle the cancellation
  /// @todo Handle NULL req

  conn->sendSuccessfulCancellationResponse(req);
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
  m_ui->jobView->setModel(m_jobItemModel);
  m_ui->jobView->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
}

} // End namespace
