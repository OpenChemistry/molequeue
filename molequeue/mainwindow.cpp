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
#include "jobactionfactories/killjobactionfactory.h"
#include "jobactionfactories/opendirectoryactionfactory.h"
#include "jobactionfactories/programmableopenwithactionfactory.h"
#include "jobactionfactories/removejobactionfactory.h"
#include "jobmanager.h"
#include "logentry.h"
#include "logger.h"
#include "logwindow.h"
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
    m_logWindow(NULL),
    m_openWithManagerDialog(NULL),
    m_queueManagerDialog(NULL),
    m_minimizeAction(NULL),
    m_maximizeAction(NULL),
    m_restoreAction(NULL),
    m_trayIcon(NULL),
    m_trayIconMenu(NULL),
    m_server(new Server (this))
{
  m_ui->setupUi(this);

  QIcon icon(":/icons/molequeue.png");
  setWindowIcon(icon);

  createActions();
  createActionFactories();
  createMainMenu();
  createTrayIcon();
  readSettings();
  createJobTable();

  connect(m_server, SIGNAL(connectionError(MoleQueue::ConnectionListener::Error,QString)),
          this, SLOT(handleServerConnectionError(MoleQueue::ConnectionListener::Error, QString)));

  connect(Logger::getInstance(), SIGNAL(newLogEntry(MoleQueue::LogEntry)),
          this, SLOT(notifyUserOfLogEntry(MoleQueue::LogEntry)));

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

  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());

  m_server->readSettings(settings);
  ActionFactoryManager::getInstance()->readSettings(settings);
}

void MainWindow::writeSettings()
{
  QSettings settings;

  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());

  m_server->writeSettings(settings);
  ActionFactoryManager::getInstance()->writeSettings(settings);
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
  if (reason != QSystemTrayIcon::Context)
    show();
}

void MainWindow::notifyUserOfLogEntry(const MoleQueue::LogEntry &entry)
{
  if (!m_trayIcon->supportsMessages())
    return;

  QString title;

  QSystemTrayIcon::MessageIcon icon;
  switch (entry.entryType()) {
  default:
  case LogEntry::DebugMessage:
    return;
  case LogEntry::Notification:
    icon = QSystemTrayIcon::Information;
    title = tr("Notification");
    break;
  case LogEntry::Warning:
    icon = QSystemTrayIcon::Warning;
    title = tr("Warning");
    break;
  case LogEntry::Error:
    icon = QSystemTrayIcon::Critical;
    title = tr("Error");
    break;
  }

  m_trayIcon->showMessage(title, entry.message(), icon);
}

void MainWindow::showQueueManagerDialog()
{
  if (!m_queueManagerDialog) {
    m_queueManagerDialog =
        new QueueManagerDialog(m_server->queueManager(), this);
  }

  m_queueManagerDialog->show();
  m_queueManagerDialog->raise();
}

void MainWindow::showOpenWithManagerDialog()
{
  if (!m_openWithManagerDialog)
    m_openWithManagerDialog = new OpenWithManagerDialog(this);

  m_openWithManagerDialog->show();
  m_openWithManagerDialog->raise();
}

void MainWindow::showLogWindow()
{
  if (m_logWindow == NULL)
    m_logWindow = new LogWindow(this);

  m_logWindow->show();
  m_logWindow->raise();
}

void MainWindow::handleServerConnectionError(ConnectionListener::Error err,
                                             const QString &str)
{
  // handle AddressInUseError by giving user option to replace current socket
  if (err == ConnectionListener::AddressInUseError) {
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
      hide();
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

void MainWindow::closeEvent(QCloseEvent *theEvent)
{
  QSettings settings;

  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());

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
          this, SLOT(showQueueManagerDialog()));
  connect(m_ui->actionOpenWithManager, SIGNAL(triggered()),
          this, SLOT(showOpenWithManagerDialog()));
  connect(m_ui->actionShowLog, SIGNAL(triggered()),
          this, SLOT(showLogWindow()));
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

  QIcon icon(":/icons/molequeue.png");
  m_trayIcon->setIcon(icon);

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

  KillJobActionFactory *killActionFactory =
      new KillJobActionFactory();
  killActionFactory->setServer(m_server);
  manager->addFactory(killActionFactory);
}

} // End namespace
