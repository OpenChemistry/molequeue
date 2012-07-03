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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "actionfactorymanager.h"
#include "job.h"
#include "jobactionfactories/programmableopenwithactionfactory.h"
#include "jobactionfactories/opendirectoryactionfactory.h"
#include "jobactionfactories/removejobactionfactory.h"
#include "jobmanager.h"
#include "openwithmanagerdialog.h"
#include "queuemanagerdialog.h"
#include "server.h"

#include <QtCore/QSettings>

#include <QtGui/QCloseEvent>
#include <QtGui/QInputDialog>
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
    m_server(new Server (this))
{
  m_ui->setupUi(this);

  createActions();
  createActionFactories();
  createMainMenu();
  createTrayIcon();
  readSettings();
  createJobTable();

  connect(m_server->jobManager(), SIGNAL(jobStateChanged(const MoleQueue::Job*,
                                                         MoleQueue::JobState,
                                                         MoleQueue::JobState)),
          this, SLOT(notifyJobStateChanged(const MoleQueue::Job*,
                                           MoleQueue::JobState,
                                           MoleQueue::JobState)));
  connect(m_server, SIGNAL(connectionError(QAbstractSocket::SocketError,QString)),
          this, SLOT(handleServerConnectionError(QAbstractSocket::SocketError, QString)));

  connect(m_server, SIGNAL(errorNotification(QString,QString)),
          this, SLOT(notifyUserOfError(QString,QString)));

  m_server->setDebug(true);
  m_server->start();

  m_trayIcon->show();
}

MainWindow::~MainWindow()
{
  writeSettings();

  delete m_ui;
  delete m_server;
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

  this->restoreGeometry(settings.value("geometry").toByteArray());
  this->restoreState(settings.value("windowState").toByteArray());

  m_server->readSettings(settings);
  ActionFactoryManager::getInstance()->readSettings(settings);
}

void MainWindow::writeSettings()
{
  QSettings settings;

  settings.setValue("geometry", this->saveGeometry());
  settings.setValue("windowState", this->saveState());

  m_server->writeSettings(settings);
  ActionFactoryManager::getInstance()->writeSettings(settings);
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
  if (reason != QSystemTrayIcon::Context)
    this->show();
}

void MainWindow::notifyUserOfError(const QString &title, const QString &message)
{
  if (m_trayIcon->supportsMessages())
    m_trayIcon->showMessage(title, message, QSystemTrayIcon::Warning);
  else
    QMessageBox::warning(this, title, message);
}

void MainWindow::showQueueManager()
{
  QueueManagerDialog dialog(m_server->queueManager(), this);
  dialog.exec();
}

void MainWindow::showOpenWithManager()
{
  OpenWithManagerDialog dialog(this);
  dialog.exec();
}

void MainWindow::handleServerConnectionError(QAbstractSocket::SocketError err,
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
      qApp->quit();
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

void MainWindow::notifyJobStateChanged(const Job *job, JobState,
                                       JobState newState)
{
  if (!job->popupOnStateChange())
    return;

  QString notification;
  if (newState == MoleQueue::Accepted)
    notification = tr("Job accepted: '%1'").arg(job->description());
  else if (newState == MoleQueue::Finished)
    notification = tr("Job finished: '%1'").arg(job->description());
  else
    return;

  m_trayIcon->showMessage(tr("MoleQueue"), notification,
                          QSystemTrayIcon::Information, 5000);
}

void MainWindow::closeEvent(QCloseEvent *theEvent)
{
  QSettings settings;

  settings.setValue("geometry", this->saveGeometry());
  settings.setValue("windowState", this->saveState());

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
  connect(m_ui->actionQueueManager, SIGNAL(triggered()),
          this, SLOT(showQueueManager()));
  connect(m_ui->actionOpenWithManager, SIGNAL(triggered()),
          this, SLOT(showOpenWithManager()));
  connect(m_ui->actionQuit, SIGNAL(triggered()),
          qApp, SLOT(quit()));
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

  connect(m_trayIcon, SIGNAL(messageClicked()),
          this, SLOT(show()));
  connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::createJobTable()
{
  m_ui->jobTableWidget->setJobManager(m_server->jobManager());
}

void MainWindow::createActionFactories()
{
  ActionFactoryManager *manager = ActionFactoryManager::getInstance();
  manager->setServer(m_server);

  // Create default factories:
  OpenDirectoryActionFactory *dirActionFactory =
      new OpenDirectoryActionFactory();
  dirActionFactory->setServer(m_server);
  manager->addFactory(dirActionFactory);

  RemoveJobActionFactory *removeActionFactory =
      new RemoveJobActionFactory();
  removeActionFactory->setServer(m_server);
  manager->addFactory(removeActionFactory);
}

} // End namespace
