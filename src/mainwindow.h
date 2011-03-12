
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QSystemTrayIcon>

class QAction;
class QIcon;
class QLocalServer;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

  void setVisible(bool visible);
  
protected:
  void closeEvent(QCloseEvent *event);
  
private slots:
  void setIcon(int index);
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void showMessage();
  void messageClicked();
  void newConnection();
  void socketReadyRead();
  
private:
  void createIconGroupBox();
  void createMessageGroupBox();
  void createActions();
  void createMainMenu();
  void createTrayIcon();
  
  /** Our MainWindow GUI. */
  Ui::MainWindow *m_ui;
  
  QAction *m_minimizeAction;
  QAction *m_maximizeAction;
  QAction *m_restoreAction;
  QAction *m_quitAction;
  
  QIcon *m_icon;
  QSystemTrayIcon *m_trayIcon;
  QMenu *m_trayIconMenu;

  QLocalServer *m_server;
};

#endif
