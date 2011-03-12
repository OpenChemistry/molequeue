#include <QtGui/QApplication>
#include <QtGui/QSystemTrayIcon>
#include <QtGui/QMessageBox>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    QMessageBox::critical(0, QObject::tr("QueueTray"),
                          QObject::tr("System tray not available on this system."));
    return 1;
  }

  QApplication::setQuitOnLastWindowClosed(false);

  MainWindow window;
  window.show();
  return app.exec();
}
