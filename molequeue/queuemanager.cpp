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

#include "queuemanager.h"

#include "queue.h"
#include "queues/local.h"
#include "queues/remote.h"
#include "queues/sge.h"
#include "server.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>

namespace MoleQueue {

QueueManager::QueueManager(Server *parentServer)
  : QObject(parentServer),
    m_server(parentServer)
{
  qRegisterMetaType<Queue*>("MoleQueue::Queue*");
  qRegisterMetaType<const Queue*>("const MoleQueue::Queue*");
}

QueueManager::~QueueManager()
{
  QList<Queue*> queueList = m_queues.values();
  m_queues.clear();
  qDeleteAll(queueList);
}

void QueueManager::readSettings(QSettings &settings)
{
  QStringList queueNameList = settings.value("queues").toStringList();

  settings.beginGroup("Queues");
  foreach (const QString &queueName, queueNameList) {
    settings.beginGroup(queueName);

    QString queueType = settings.value("type").toString();

    Queue *queue = NULL;
    if (queueType == "Local")
      queue = new QueueLocal (this);
    else if (queueType == "Sun Grid Engine")
      queue = new QueueSge (this);
    else
      qWarning() << Q_FUNC_INFO << "Unrecognized Queue type:" << queueType;

    if (queue != NULL) {
      queue->setName(queueName);
      queue->readSettings(settings);
      this->addQueue(queue);
    }
    settings.endGroup(); // queueName
  }
  settings.endGroup(); // "Queues"
}

void QueueManager::writeSettings(QSettings &settings) const
{
  settings.setValue("queues", this->queueNames());

  settings.beginGroup("Queues");
  foreach (const Queue* queue, this->queues()) {
    settings.beginGroup(queue->name());
    settings.setValue("type", queue->typeName());
    queue->writeSettings(settings);
    settings.endGroup(); // queue->name()
  }
  settings.endGroup(); // "Queues"
}

bool QueueManager::addQueue(Queue *queue, bool replace)
{
  if (m_queues.contains(queue->name())) {
    if (replace == true)
      m_queues.take(queue->name())->deleteLater();
    else
      return false;
  }

  m_queues.insert(queue->name(), queue);

  emit queueAdded(queue->name(), queue);
  return true;
}

bool QueueManager::removeQueue(const Queue *queue)
{
  if (!m_queues.contains(queue->name()))
    return false;

  Queue *mutableQueue = m_queues.take(queue->name());

  emit queueRemoved(queue->name(), mutableQueue);
  return true;
}

bool QueueManager::removeQueue(const QString &name)
{
  if (!m_queues.contains(name))
    return false;

  Queue *queue = m_queues.take(name);

  emit queueRemoved(name, queue);
  return true;
}

QueueListType QueueManager::toQueueList() const
{
  QueueListType queueList;
  foreach(const Queue *queue, m_queues)
    queueList.insert(queue->name(), queue->programNames());

  return queueList;
}

} // end MoleQueue namespace
