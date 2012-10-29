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

#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

#include <QtGui/QApplication>
#include <QtGui/QSystemTrayIcon>
#include <QtGui/QMessageBox>

#include "mainwindow.h"

#include <cstdio>

void printVersion();
void printUsage();
bool setWorkDir(const QString &workDir);

int main(int argc, char *argv[])
{
  QCoreApplication::setOrganizationName("Kitware");
  QCoreApplication::setOrganizationDomain("kitware.com");
  QCoreApplication::setApplicationName("MoleQueue");
  QCoreApplication::setApplicationVersion("0.2.0");
  
  QApplication app(argc, argv);

  bool customWorkDirSet = false;
  bool enableRpcKill = false;
  QString socketName("MoleQueue");

  QStringList args = QCoreApplication::arguments();
  for (QStringList::const_iterator it = args.constBegin() + 1,
       itEnd = args.constEnd(); it != itEnd; ++it) {
    if (*it == "-w" || *it == "--workdir") {
      if (customWorkDirSet) {
        qWarning("%s",
                 qPrintable(QObject::tr("More than one -w or -W option set. "
                                        "Cannot run in multiple working "
                                        "directories!")));
        return EXIT_FAILURE;
      }
      if (!setWorkDir(it + 1 == itEnd ? QString("") : *(++it))) {
        return EXIT_FAILURE;
      }
      customWorkDirSet = true;
      continue;
    }
    else if (*it == "-s" || *it == "--socketname") {
      // Wait until all options are parsed in case we end up changing the
      // settings location.
      if (it + 1 == itEnd || (it+1)->isEmpty()) {
        qWarning("%s", qPrintable(QObject::tr("Missing socket name!")));
        return EXIT_FAILURE;
      }
      socketName = (*++it);
      continue;
    }
    else if (*it == "-v" || *it == "--version") {
      printVersion();
      return EXIT_SUCCESS;
    }
    else if (*it == "--rpc-kill") {
      enableRpcKill = true;
      continue;
    }
    else if (*it == "-h" || *it == "-H" || *it == "--help" || *it == "-help") {
      printUsage();
      return EXIT_SUCCESS;
    }
    else {
      qWarning(qPrintable(QObject::tr("Unrecognized command line option: %s")),
               qPrintable(*it));
      printUsage();
      return EXIT_FAILURE;
    }
  }

  // The QSettings object here will point to either the standard config file
  // or the one set by --workdir. Update any configuration info here:
  QSettings settings;
  settings.setValue("socketName", socketName);
  settings.setValue("enableRpcKill", enableRpcKill);

  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    QMessageBox::critical(0, QObject::tr("MoleQueue"),
                          QObject::tr("System tray not available on this "
                                      "system."));
    return EXIT_FAILURE;
  }

  QApplication::setQuitOnLastWindowClosed(false);

  // Send a signal to the mainwindow. This will get handled when the event loop
  // starts and will launch the server. This must be done this way, otherwise
  // the user may decide to quit the application if the socket is already
  // in use, and the call to qApp->exit() will be ignored if no event loop is
  // running.
  MoleQueue::MainWindow window;
  QTimer::singleShot(0, &window, SLOT(onEventLoopStart()));
  return app.exec();
}

void printVersion()
{
  qWarning("%s %s", qPrintable(qApp->applicationName()),
           qPrintable(qApp->applicationVersion()));
}

void printUsage()
{
  printVersion();
  qWarning("%s\n\n%s",
           qPrintable(QObject::tr("Usage: molequeue [options]")),
           qPrintable(QObject::tr("Options:")));

  const char *format = "      %3s %-20s   %s";

  qWarning(format, "-h,", "--help",
           qPrintable(QObject::tr("Print version and usage information and "
                                  "exit.")));
  qWarning(format, "", "--rpc-kill",
           qPrintable(QObject::tr("Allow the app to be killed by a special "
                                  "RPC call (testing only).")));
  qWarning(format, "-s,", "--socketname [name]",
           qPrintable(QObject::tr("Name of the socket on which to listen.")));
  qWarning(format, "-v,", "--version",
           qPrintable(QObject::tr("Print version information and exit.")));
  qWarning(format, "-w,", "--workdir [path]",
           qPrintable(QObject::tr("Run MoleQueue in a custom working "
                                  "directory.")));
}

bool setWorkDir(const QString &workDir)
{
  if (workDir.isEmpty()) {
    qWarning("%s", qPrintable(QObject::tr("No specified working directory!")));
    return false;
  }

  QFileInfo dirInfo(QDir::cleanPath(workDir));
  if (!dirInfo.exists()) {
    qWarning(qPrintable(QObject::tr("Specified working directory does not "
                                    "exist: '%s'")), qPrintable(workDir));
    return false;
  }
  if (!dirInfo.isReadable()) {
    qWarning(qPrintable(QObject::tr("Specified working directory is not "
                                    "readable: '%s'")), qPrintable(workDir));
    return false;
  }
  if (!dirInfo.isWritable()) {
    qWarning(qPrintable(QObject::tr("Specified working directory is not "
                                    "writable: '%s'")), qPrintable(workDir));
    return false;
  }

  QDir dir(workDir);

  if (dir.exists("config")) {
    QFileInfo configInfo(dir, "config");
    if (!configInfo.isDir()) {
      qWarning(qPrintable(QObject::tr("Invalid working directory '%s': "
                                      "'%s/config' exists and is not a "
                                      "directory!")), qPrintable(workDir),
                                      qPrintable(workDir));
      return false;
    }
    if (!configInfo.isReadable()) {
      qWarning(qPrintable(QObject::tr("Invalid working directory '%s': "
                                      "'%s/config' is not readable!")),
                                      qPrintable(workDir), qPrintable(workDir));
      return false;
    }
    if (!configInfo.isWritable()) {
      qWarning(qPrintable(QObject::tr("Invalid working directory '%s': "
                                      "'%s/config' is not writable!")),
                                      qPrintable(workDir), qPrintable(workDir));
      return false;
    }
  }
  else {
    if (!dir.mkdir("config")) {
      qWarning(qPrintable(QObject::tr("Cannot create directory '%s'. "
                                      "Aborting.")), qPrintable(workDir));
      return false;
    }
  }

  qDebug(qPrintable(QObject::tr("Running in working directory '%s'...")),
         qPrintable(dirInfo.absolutePath()));

  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                     workDir + "/config/");
  QSettings settings;
  settings.setValue("workingDirectoryBase", workDir);

  return true;
}
