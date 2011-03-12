#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QTimer>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

MainWindow::MainWindow() : m_removeServer(false)
{
  m_ui = new Ui::MainWindow;
  m_ui->setupUi(this);

  createActions();
  createMainMenu();
  createTrayIcon();

  m_trayIcon->show();

  setWindowTitle(tr("MoleQueue"));
  resize(400, 300);

  // Start up our local socket server
  m_server = new QLocalServer(this);
  if (!m_server->listen("MoleQueue")) {
    QMessageBox::critical(this, tr("MoleQueue Server"),
                                tr("Unable to start the server: %1.\n'%2'")
                                  .arg(m_server->errorString())
                          .arg(m_server->fullServerName()));
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
    qDebug() << "Connecting server new connection up...";
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
  clientSocket->write("Hello");
  clientSocket->flush();
  clientSocket->disconnectFromServer();

  // Experimental QProcess for ssh
  qDebug() << "Calling SSH...";
  QProcess ssh;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("SSH_ASKPASS", "/usr/bin/pinentry-qt4");
  ssh.setProcessEnvironment(env);
  ssh.start("env");
  //ssh.start("ssh", QStringList() << "unobtanium" << "ls");
  if (!ssh.waitForStarted()) {
    qDebug() << "Failed to start SSH...";
    return;
  }
//  ssh.write("ls ~/");
//  ssh.write("exit");
  ssh.closeWriteChannel();
  if (!ssh.waitForFinished()) {
    ssh.close();
    qDebug() << "Failed to exit.";
  }
  QByteArray result = ssh.readAll();
  qDebug() << "Output:" << result;
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

  m_icon = new QIcon(":icons/avogadro.png");
  m_trayIcon->setIcon(*m_icon);

  if (m_trayIcon->supportsMessages())
    m_trayIcon->setToolTip("Queue manager...");
  else
    m_trayIcon->setToolTip("Queue manager (no message support)...");

  m_trayIcon->showMessage("info",
                          "System tray resident queue manager initialized.");
}
