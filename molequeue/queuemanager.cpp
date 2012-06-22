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
#include "server.h"

// Concrete queue classes
#include "queues/local.h"
#include "queues/pbs.h"
#include "queues/sge.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>

namespace MoleQueue {

QueueManager::QueueManager(Server *parentServer)
  : Object(parentServer),
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

    Queue *queue = this->addQueue(queueName, queueType, this);

    if (queue != NULL)
      queue->readSettings(settings);
    else
      qWarning() << Q_FUNC_INFO << "Unrecognized Queue type:" << queueType;

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

const QStringList &QueueManager::availableQueues()
{
  static QStringList result;
  if (result.isEmpty())
    result << "Local" << "Sun Grid Engine" << "PBS/Torque";
  return result;
}

bool QueueManager::queueTypeIsValid(const QString &queueType)
{
  return QueueManager::availableQueues().contains(queueType);
}

Queue * QueueManager::addQueue(
    const QString &queueName, const QString &queueType, bool replace)
{
  if (m_queues.contains(queueName)) {
    if (replace == true)
      m_queues.take(queueName)->deleteLater();
    else
      return NULL;
  }

  Queue * newQueue = NULL;
  if (queueType== "Local")
    newQueue = new QueueLocal (this);
  else if (queueType== "Sun Grid Engine")
    newQueue = new QueueSge (this);
  else if (queueType== "PBS/Torque")
    newQueue = new QueuePbs (this);

  if (!newQueue)
    return NULL;

  newQueue->setName(queueName);

  m_queues.insert(newQueue->name(), newQueue);
  emit queueAdded(newQueue->name(), newQueue);
  return newQueue;
}

bool QueueManager::removeQueue(const Queue *queue)
{
  return this->removeQueue(queue->name());
}

bool QueueManager::removeQueue(const QString &name)
{
  if (!m_queues.contains(name))
    return false;

  Queue *queue = m_queues.take(name);

  emit queueRemoved(name, queue);
  queue->deleteLater();
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
