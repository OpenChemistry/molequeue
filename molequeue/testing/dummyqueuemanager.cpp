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

#include "dummyqueuemanager.h"

#include "dummyqueueremote.h"

using namespace MoleQueue;

DummyQueueManager::DummyQueueManager(Server *parentServer)
  : MoleQueue::QueueManager(parentServer)
{
}

DummyQueueManager::~DummyQueueManager()
{
}

Queue *DummyQueueManager::addQueue(const QString &queueName,
                                   const QString &queueType, bool replace)
{
  if (m_queues.contains(queueName)) {
    if (replace == true)
      m_queues.take(queueName)->deleteLater();
    else
      return NULL;
  }

  Queue * newQueue = NULL;
  if (queueType == "Dummy")
    newQueue = new DummyQueueRemote(queueName, this);

  if (!newQueue)
    return NULL;

  m_queues.insert(newQueue->name(), newQueue);
  emit queueAdded(newQueue->name(), newQueue);
  return newQueue;
}
