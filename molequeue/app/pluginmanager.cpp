/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "pluginmanager.h"

#include "molequeueconfig.h"

#include <molequeue/servercore/connectionlistenerfactory.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QMutex>
#include <QtCore/QPluginLoader>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <QtCore/QDebug>

namespace MoleQueue
{

namespace
{
// Compiler initializes this static pointer to 0.
static PluginManager *pluginManagerInstance;
}

PluginManager::PluginManager(QObject *p) : QObject(p)
{
  QString libDir(MoleQueue_LIB_DIR);
  m_relativeToApp = "/../" + libDir + "/molequeue/plugins";
#ifdef __APPLE__
  QString buildRelative("/../../../../");
  m_relativeToApp = buildRelative + m_relativeToApp;
  qDebug() << QCoreApplication::applicationDirPath() + buildRelative
              + "/CMakeCache.txt";
  if (QFileInfo(QCoreApplication::applicationDirPath() + buildRelative
                + "/CMakeCache.txt").exists()) {
    qDebug() << QCoreApplication::applicationDirPath()
                + buildRelative
                + libDir + "/molequeue/plugins";
    m_pluginDirs.append(QDir(QCoreApplication::applicationDirPath()
                             + buildRelative
                             + libDir + "/molequeue/plugins").absolutePath());
    qDebug() << QDir(QCoreApplication::applicationDirPath()
                     + buildRelative
                     + libDir + "/molequeue/plugins").absolutePath();
  }
#endif

// For multi configuration types add correct one to search path.
#ifdef MULTI_CONFIG_BUILD
  // First extract the build type (the name of the application dir).
  QDir appDir(QCoreApplication::applicationDirPath());
  QString buildType = appDir.dirName();

  QDir condir(QCoreApplication::applicationDirPath()
           + "/../../" + libDir + "/molequeue/plugins/"+ buildType);
  m_pluginDirs.append(condir.absolutePath());
#endif
  QDir dir(QCoreApplication::applicationDirPath() + m_relativeToApp);
  m_pluginDirs.append(dir.absolutePath());
}

PluginManager::~PluginManager()
{
}

PluginManager * PluginManager::instance()
{
  static QMutex mutex;
  if (!pluginManagerInstance) {
    mutex.lock();
    if (!pluginManagerInstance)
      pluginManagerInstance = new PluginManager(QCoreApplication::instance());
    mutex.unlock();
  }
  return pluginManagerInstance;
}

void PluginManager::load()
{
  foreach(const QString &dir, m_pluginDirs)
    load(dir);
}

void PluginManager::load(const QString &path)
{
  QDir dir(path);
  qDebug() << "Checking for plugins in" << path;
  qDebug() << dir.entryList(QDir::Files);
  foreach(const QString &pluginPath, dir.entryList(QDir::Files)) {
    QPluginLoader pluginLoader(dir.absolutePath() + QDir::separator() + pluginPath);

    if (pluginLoader.isLoaded()) {
      qDebug() << "Plugin already loaded: " << pluginLoader.fileName();
      continue;
    }

    QObject *pluginInstance = pluginLoader.instance();

    // Check if the plugin loaded correctly. Keep debug output for now, should
    // go away once we have verified this (or added to a logger).
    if (!pluginInstance) {
      qDebug() << "Failed to load" << pluginPath << "error"
               << pluginLoader.errorString();
    }
    else {
      qDebug() << "Loaded" << pluginPath << "->";
      pluginInstance->dumpObjectInfo();
    }

    // Now attempt to cast to known factory types, and make it available.
    ConnectionListenerFactory *connectionListenerFactory =
      qobject_cast<ConnectionListenerFactory *>(pluginInstance);
    if (connectionListenerFactory &&
        !m_connectionListenerFactories.contains(connectionListenerFactory))
      m_connectionListenerFactories.append(connectionListenerFactory);
  }
}

QList<ConnectionListenerFactory *> PluginManager::connectionListenerFactories() const
{
  return m_connectionListenerFactories;
}

} // End MoleQueue namespace
