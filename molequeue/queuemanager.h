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

#include "object.h"

#include "molequeueglobal.h"

#include <QtCore/QMap>

class QSettings;

namespace MoleQueue
{
class Queue;
class Server;

class QueueManager : public Object
{
  Q_OBJECT

public:
  explicit QueueManager(Server *parentServer = 0);
  ~QueueManager();

  /// @param settings QSettings object to write state to.
  void readSettings(QSettings &settings);
  /// @param settings QSettings object to read state from.
  void writeSettings(QSettings &settings) const;

  /// @return The parent Server
  Server *server() {return m_server;}
  /// @return The parent Server
  const Server *server() const {return m_server;}

  /**
   * @param name String containing the name of the queue of interest.
   * @return The requested Queue, or NULL if none exist with that name.
   */
  Queue * lookupQueue(const QString &name) const
  {
    return m_queues.value(name, NULL);
  }

  /**
   * @return A list of available queues types (e.g. PBS/Torque, SGE, etc.)
   */
  static const QStringList & availableQueues();

  /**
   * @param queueType Type of Queue (SGE, PBS/Torque, Local, etc)
   * @return True if the queue can be instantiated, false otherwise.
   */
  static bool queueTypeIsValid(const QString &queueType);

  /**
   * Add a new Queue to the QueueManager. The new queueName must be unique name.
   * The QueueManager maintains ownership of the Queue.
   * @param queueName Unique, user-visible name of the new Queue object.
   * @param queueType The type of the new Queue object, e.g. PBS/Torque, SGE, etc/
   * @param replace Defaults to false; if true, replace any existing queues with
   * the same name. The old queue with the same name will be deleted.
   * @return A pointer to the new queue if successful, NULL otherwise.
   * @sa queueTypeIsKnown
   * @sa availableQueueTypes
   */
  Queue * addQueue(const QString &queueName, const QString &queueType,
                   bool replace = false);

  /**
   * Remove and delete a queue from the collection.
   * @param queue Queue to remove.
   * @return True if queue exists, false otherwise.
   */
  bool removeQueue(const Queue *queue);

  /**
   * Remove and delete a queue from the collection.
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

protected:
  QMap<QString, Queue*> m_queues;
  Server *m_server;
};

} // end MoleQueue namespace

#endif // QUEUEMANAGER_H
