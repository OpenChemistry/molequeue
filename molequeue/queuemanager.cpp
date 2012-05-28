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

namespace MoleQueue {

QueueManager::QueueManager(QObject *parentObject)
  : QObject(parentObject)
{
  qRegisterMetaType<Queue*>("Queue*");
  qRegisterMetaType<const Queue*>("const Queue*");
}

QueueManager::~QueueManager()
{
  QList<Queue*> queueList = m_queues.values();
  m_queues.clear();
  qDeleteAll(queueList);
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
  foreach(const Queue *queue, m_queues) {
    queueList.insert(queue->name(), queue->programNames());
  }

  return queueList;
}

} // end MoleQueue namespace
