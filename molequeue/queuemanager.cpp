/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "queuemanager.h"

#include "queues/local.h"
#include "queues/remote.h"
#include "queues/sge.h"

namespace MoleQueue {

QueueManager::QueueManager(QObject *parentObject)
  : QObject(parentObject)
{
}

QueueManager::~QueueManager()
{
}

void QueueManager::addQueue(Queue *queue)
{
  m_queues.append(queue);

  emit queueAdded(queue);
}

void QueueManager::addQueue(const QString &type)
{
  Queue *queue = createQueue(type);

  if(queue){
    addQueue(queue);
  }
}

void QueueManager::removeQueue(Queue *queue)
{
  m_queues.removeAll(queue);

  emit queueRemoved(queue);
}

Queue* QueueManager::createQueue(const QString &type)
{
  if(type == "Local")
    return new QueueLocal(this);
  else if(type == "Remote")
    return new QueueRemote(this);
  else if(type == "Remote - SGE")
    return new QueueSGE(this);
  else
    return 0;
}

QStringList QueueManager::queueTypes() const
{
  QStringList types;

  types.append("Local");
  types.append("Remote");
  types.append("Remote - SGE");

  return types;
}

QueueListType QueueManager::toQueueList() const
{
  QueueListType queueList;
  QString queueName;
  QStringList programList;
  foreach(const Queue *queue, m_queues) {
    queueName = queue->name();

    programList.clear();
    foreach(const QString progName, queue->programs()) {
      programList << progName;
    }

    queueList.insert(queueName, programList);
  }

  return queueList;
}

} // end MoleQueue namespace
