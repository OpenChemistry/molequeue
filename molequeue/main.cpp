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
  QStringList args = QCoreApplication::arguments();
  for (QStringList::const_iterator it = args.constBegin() + 1,
       it_end = args.constEnd(); it != it_end; ++it) {
    if (*it == "-w" || *it == "--workdir") {
      if (customWorkDirSet) {
        printf("More than one -w or -W option set. Cannot run in multiple "
               "working directories!\n");
        return EXIT_FAILURE;
      }
      if (!setWorkDir(it + 1 == it_end ? QString("") : *(++it))) {
        return EXIT_FAILURE;
      }
      customWorkDirSet = true;
      continue;
    }
    else if (*it == "-v" || *it == "--version") {
      printVersion();
      return EXIT_SUCCESS;
    }
    else if (*it == "-h" || *it == "-H" || *it == "--help" || *it == "-help") {
      printUsage();
      return EXIT_SUCCESS;
    }
    else {
      QByteArray ba = it->toLatin1();
      printf("Unrecognized command line option: '%s'\n", ba.constData());
      printUsage();
      return EXIT_FAILURE;
    }
  }

  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    QMessageBox::critical(0, QObject::tr("MoleQueue"),
                          QObject::tr("System tray not available on this system."));
    return EXIT_FAILURE;
  }

  QApplication::setQuitOnLastWindowClosed(false);

  MoleQueue::MainWindow window;
  window.show();
  return app.exec();
}

void printVersion()
{
  QByteArray appName = QCoreApplication::applicationName().toLatin1();
  QByteArray ba = QCoreApplication::applicationVersion().toLatin1();
  printf("%s %s\n", appName.constData(), ba.constData());
}

void printUsage()
{
  printVersion();
  printf("Usage: molequeue [options]\n\nOptions:\n");

  const char *fmt = "      %3s %-20s   %s\n";

  printf(fmt, "-h,", "--help",
         "Print version and usage information and exit.");
  printf(fmt, "-v,", "--version",
         "Print version information and exit.");
  printf(fmt, "-w,", "--workdir [path]",
         "Run MoleQueue in a custom working directory.");
}

bool setWorkDir(const QString &workDir)
{
  QByteArray ba = workDir.toLatin1();

  QFileInfo dirInfo(workDir);
  if (!dirInfo.exists()) {
    printf("Specified working directory does not exist: '%s'\n",
           ba.constData());
    return false;
  }
  if (!dirInfo.isReadable()) {
    printf("Specified working directory is not readable: '%s'\n",
           ba.constData());
    return false;
  }
  if (!dirInfo.isWritable()) {
    printf("Specified working directory is not writable: '%s'\n",
           ba.constData());
    return false;
  }

  QDir dir(workDir);

  if (dir.exists("config")) {
    QFileInfo configInfo(dir, "config");
    if (!configInfo.isDir()) {
      printf("Invalid working directory '%s': '%s/config' exists and is not a "
             "directory!\n", ba.constData(), ba.constData());
      return false;
    }
    if (!configInfo.isReadable()) {
      printf("Invalid working directory '%s': '%s/config' is not readable!\n",
             ba.constData(), ba.constData());
      return false;
    }
    if (!configInfo.isWritable()) {
      printf("Invalid working directory '%s': '%s/config' is not writable!\n",
             ba.constData(), ba.constData());
      return false;
    }
  }
  else {
    if (!dir.mkdir("config")) {
      printf("Cannot create directory '%s'. Aborting.", ba.constData());
      return false;
    }
  }

  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                     workDir + "/config/");
  QSettings settings;
  settings.setValue("workingDirectoryBase", workDir);

  return true;
}
