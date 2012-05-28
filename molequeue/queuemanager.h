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

#ifndef QUEUEMANAGER_H
#define QUEUEMANAGER_H

#include <QtCore/QObject>

#include "molequeueglobal.h"

#include <QtCore/QHash>

namespace MoleQueue
{

class Queue;

class QueueManager : public QObject
{
  Q_OBJECT

public:
  explicit QueueManager(QObject *parentObject = 0);
  ~QueueManager();

  /**
   * @param name String containing the name of the queue of interest.
   * @return The requested Queue, or NULL if none exist with that name.
   */
  Queue * getQueue(const QString &name) const
  {
    return m_queues.value(name, NULL);
  }

  /**
   * Add a new Queue to the QueueManager. The new object must have a unique name.
   * The QueueManager takes ownership of the Queue unless removeQueue is called.
   * @param queue The new Queue object.
   * @return True on success, false if a Queue already exists with the same name.
   */
  bool addQueue(Queue *queue);

  /**
   * Remove a queue from the collection.
   * @param queue Queue to remove.
   * @return True if queue exists, false otherwise.
   */
  bool removeQueue(const Queue *queue);

  /**
   * Remove a queue from the collection.
   * @param queueName Name of queue to remove.
   * @return True if queue exists, false otherwise.
   */
  bool removeQueue(const QString &name);

  /**
   * @return A list of all Queue objects in the QueueManager.
   */
  QList<Queue *> queues() const
  {
    return m_queues.values();
  }

  /**
   * @return Names of all Queue objects known to the QueueManager.
   */
  QStringList queueNames() const
  {
    return m_queues.keys();
  }

  /**
   * @return The number of Queue objects known to the QueueManager.
   */
  int numQueues() const
  {
    return m_queues.size();
  }

  /**
   * @return A QueueListType container describing the queues and their
   * associated programs.
   */
  QueueListType toQueueList() const;

signals:
  /**
   * Emitted when a new Queue is added to the QueueManager
   * @param name Name of the new Queue
   * @param queue Pointer to the Queue.
   */
  void queueAdded(const QString &name, MoleQueue::Queue *queue);

  /**
   * Emitted when a Queue is removed from the QueueManager
   * @param name Name of the new Queue
   * @param queue Pointer to the Queue.
   */
  void queueRemoved(const QString &name, MoleQueue::Queue *queue);

private:
  QHash<QString, Queue*> m_queues;
};

} // end MoleQueue namespace

#endif // QUEUEMANAGER_H
