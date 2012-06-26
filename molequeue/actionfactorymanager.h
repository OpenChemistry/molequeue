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

class ActionFactoryManager : public QObject
{
  Q_OBJECT
public:
  static ActionFactoryManager * getInstance();
  virtual ~ActionFactoryManager();

  void readSettings(QSettings &settings);
  void writeSettings(QSettings &settings) const;

  void setServer(Server *s) { m_server = s; }
  Server * server() const { return m_server; }

  void addFactory(JobActionFactory *);

  const QList<JobActionFactory*> &getFactories() const;
  QList<JobActionFactory*> getFactories();
  QList<JobActionFactory*> getFactories(JobActionFactory::Flags flags) const;

  void removeFactory(JobActionFactory *factory);

protected:
  explicit ActionFactoryManager();
  static ActionFactoryManager *m_instance;
  Server *m_server;

  QList<JobActionFactory*> m_factories;

};

} // namespace MoleQueue

#endif // MOLEQUEUE_ACTIONFACTORYMANAGER_H
