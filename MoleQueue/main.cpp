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

#include <QtGui/QApplication>
#include <QtGui/QSystemTrayIcon>
#include <QtGui/QMessageBox>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QCoreApplication::setOrganizationName("Kitware");
  QCoreApplication::setOrganizationDomain("kitware.com");
  QCoreApplication::setApplicationName("MoleQueue");
  QCoreApplication::setApplicationVersion("0.2.0");
  
  QApplication app(argc, argv);

  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    QMessageBox::critical(0, QObject::tr("MoleQueue"),
                          QObject::tr("System tray not available on this system."));
    return 1;
  }

  QApplication::setQuitOnLastWindowClosed(false);

  MoleQueue::MainWindow window;
  window.show();
  return app.exec();
}
