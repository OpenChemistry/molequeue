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

#include "jobactionfactories/programmableopenwithactionfactory.h"

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
      settings.beginReadArray("ProgrammableOpenWithActionFactories");

  for (int i = 0; i < numFactories; ++i) {
    settings.setArrayIndex(i);
    ProgrammableOpenWithActionFactory *newFactory =
        new ProgrammableOpenWithActionFactory;
    newFactory->setServer(m_server);
    newFactory->readSettings(settings);
    this->addFactory(newFactory);
  }

  settings.endArray();

  settings.endGroup();
}

void ActionFactoryManager::writeSettings(QSettings &settings) const
{
  settings.beginGroup("ActionFactoryManager");

  QList<JobActionFactory*> progFactories =
      this->getFactories(JobActionFactory::ProgrammableOpenWith);

  settings.beginWriteArray("ProgrammableOpenWithActionFactories",
                           progFactories.size());

  for (int i = 0; i < progFactories.size(); ++i) {
    settings.setArrayIndex(i);
    static_cast<ProgrammableOpenWithActionFactory*>(
          progFactories[i])->writeSettings(settings);
  }

  settings.endArray();

  settings.endGroup();
}

ActionFactoryManager *ActionFactoryManager::getInstance()
{
  if (!m_instance) {
    m_instance = new ActionFactoryManager;
  }

  return m_instance;
}

void ActionFactoryManager::addFactory(JobActionFactory *newFactory)
{
  if (!m_factories.contains(newFactory)) {
    newFactory->setServer(this->server());
    m_factories.append(newFactory);
  }
}

QList<JobActionFactory *> ActionFactoryManager::getFactories() const
{
  return m_factories;
}

QList<JobActionFactory *> ActionFactoryManager::getFactories(JobActionFactory::Flags flags) const
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
