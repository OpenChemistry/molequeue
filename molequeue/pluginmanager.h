/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QList>

namespace MoleQueue
{

class ConnectionListenerFactory;

/*!
 * \class PluginManager pluginmanager.h <avogadro/qtgui/pluginmanager.h>
 * \brief This class takes care of finding and loading MoleQueue plugins.
 *
 * This class will find and load MoleQueue plugins. Once loaded you can use an
 * instance of this class to query and construct plugin instances. By default
 * plugins are loaded from
 * QApplication::applicationDirPath()../lib/molequeue/plugins but this can be
 * changed or more paths can be added.
 */

class PluginManager : public QObject
{
  Q_OBJECT

public:
  /*! Get the singleton instance of the plugin manager. This instance should not
   * be deleted.
   */
  static PluginManager * instance();

  /*! Get a reference to the plugin directory path list. Modifying this before
   * calling load will allow you to add, remove or append to the search paths.
   */
  QStringList& pluginDirList() { return m_pluginDirs; }

  /*! Load all plugins available in the specified plugin directories. */
  void load();
  void load(const QString &dir);

  /*! Return the loaded connection listener factories. Will be empty unless load was
   * already called.
   */
  QList<ConnectionListenerFactory *> connectionListenerFactories() const;

private:
  // Hide the constructor, destructor, copy and assignment operator.
  PluginManager(QObject *parent = 0);
  ~PluginManager();
  PluginManager(const PluginManager&);            // Not implemented.
  PluginManager& operator=(const PluginManager&); // Not implemented.

  QStringList m_pluginDirs;
  QString     m_relativeToApp;

  // Various factories loaded by the plugin manager.
  QList<ConnectionListenerFactory *> m_connectionListenerFactories;
};

} // End MoleQueue namespace

#endif // PLUGINMANAGER_H
