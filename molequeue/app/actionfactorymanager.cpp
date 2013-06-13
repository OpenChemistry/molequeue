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

#include "actionfactorymanager.h"

#include "jobactionfactories/openwithactionfactory.h"

namespace MoleQueue {

ActionFactoryManager *ActionFactoryManager::m_instance = NULL;

ActionFactoryManager::ActionFactoryManager() :
  QObject(NULL),
  m_server(NULL)
{
}

ActionFactoryManager::~ActionFactoryManager()
{
  qDeleteAll(m_factories);
  m_instance = NULL;
}

void ActionFactoryManager::readSettings(QSettings &settings)
{
  settings.beginGroup("ActionFactoryManager");

  int numFactories =
      settings.beginReadArray("openWithActionFactories");
  for (int i = 0; i < numFactories; ++i) {
    settings.setArrayIndex(i);
    OpenWithActionFactory *newFactory = new OpenWithActionFactory;
    newFactory->readSettings(settings);
    addFactory(newFactory);
  }
  settings.endArray();

  settings.endGroup();
}

void ActionFactoryManager::writeSettings(QSettings &settings) const
{
  settings.beginGroup("ActionFactoryManager");

  QList<OpenWithActionFactory*> factoryList =
      factoriesOfType<OpenWithActionFactory>();

  settings.beginWriteArray("openWithActionFactories",
                           factoryList.size());
  for (int i = 0; i < factoryList.size(); ++i) {
    settings.setArrayIndex(i);
    factoryList[i]->writeSettings(settings);
  }
  settings.endArray();

  settings.endGroup();
}

ActionFactoryManager *ActionFactoryManager::instance()
{
  if (!m_instance) {
    m_instance = new ActionFactoryManager;
  }

  return m_instance;
}

void ActionFactoryManager::addFactory(JobActionFactory *newFactory)
{
  if (!m_factories.contains(newFactory)) {
    newFactory->setServer(server());
    m_factories.append(newFactory);
  }
}

QList<JobActionFactory *> ActionFactoryManager::factories() const
{
  return m_factories;
}

QList<JobActionFactory *>
ActionFactoryManager::factories(JobActionFactory::Flags flags) const
{
  QList<JobActionFactory *> result;
  foreach (JobActionFactory *factory, m_factories) {
    if ((factory->flags() & flags) == flags)
      result << factory;
  }
  return result;
}

void ActionFactoryManager::removeFactory(JobActionFactory *factory)
{
  m_factories.removeOne(factory);
  factory->deleteLater();
}

} // namespace MoleQueue
