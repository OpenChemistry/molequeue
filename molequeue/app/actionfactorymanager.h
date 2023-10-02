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

#ifndef MOLEQUEUE_ACTIONFACTORYMANAGER_H
#define MOLEQUEUE_ACTIONFACTORYMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

#include "jobactionfactory.h"

namespace MoleQueue
{
class Server;

/**
 * @class ActionFactoryManager actionfactorymanager.h
 * <molequeue/actionfactorymanager.h>
 * @brief Singleton container for JobActionFactory objects.
 * @author Allison Vacanti
 */
class ActionFactoryManager : public QObject
{
  Q_OBJECT
public:
  /**
   * @return The singleton instance of the manager.
   */
  static ActionFactoryManager * instance();

  ~ActionFactoryManager();

  /** Read programmatically constructed factories from @a settings. */
  void readSettings(QSettings &settings);

  /** Write programmatically constructed factories to @a settings. */
  void writeSettings(QSettings &settings) const;

  /** Set the Server object used by owned factories. */
  void setServer(Server *s) { m_server = s; }

  /** Get the Server object used by owned factories. */
  Server * server() const { return m_server; }

  /**
   * Add a factory to the manager. The Manager takes ownership of the factory
   * and sets the server ivar of the factor to the
   * ActionFactoryManager::server() instance.
   */
  void addFactory(JobActionFactory *);

  /**
   * @return A list of all factories owned by the manager.
   */
  QList<JobActionFactory*> factories() const;

  /**
   * Obtain a subset of owned factories. Factories whose
   * JobActionFactory::flags() method returns a superset of @a flags will be
   * returned.
   * @param flags A combination of JobActionFactory::Flags used to filter the
   * returned list.
   * @return A list of JobActionFactory pointers filtered by @a flags.
   */
  QList<JobActionFactory*> factories(JobActionFactory::Flags flags) const;

  /**
   * Get all factories of a specific type.
   * @param FactoryType A subclass of JobActionFactory.
   * @return A list of FactoryType pointers.
   */
  template <class FactoryType>
  QList<FactoryType*> factoriesOfType() const
  {
    QList<FactoryType*> result;
    foreach (JobActionFactory *factory, m_factories) {
      if (FactoryType *f = qobject_cast<FactoryType*>(factory)) {
        result << f;
      }
    }
    return result;
  }

  /**
   * Remove the factory pointed to by @a factory from the manager.
   */
  void removeFactory(JobActionFactory *factory);

protected:
  ActionFactoryManager();
  static ActionFactoryManager *m_instance;
  Server *m_server;

  QList<JobActionFactory*> m_factories;

};

} // namespace MoleQueue

#endif // MOLEQUEUE_ACTIONFACTORYMANAGER_H
