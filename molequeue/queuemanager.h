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

#ifndef QUEUEMANAGER_H
#define QUEUEMANAGER_H

#include <QtCore>

#include "molequeueglobal.h"

namespace MoleQueue {

class Queue;

class QueueManager : public QObject
{
  Q_OBJECT

public:
  explicit QueueManager(QObject *parentObject = 0);
  ~QueueManager();

  void addQueue(Queue *queue);
  void addQueue(const QString &type);
  void removeQueue(Queue *queue);
  QList<Queue *> queues() const { return m_queues; }
  size_t queueCount() const { return m_queues.size(); }

  Queue* createQueue(const QString &type);
  QStringList queueTypes() const;

  QueueListType toQueueList() const;

signals:
  void queueAdded(const MoleQueue::Queue *queue);
  void queueRemoved(const MoleQueue::Queue *queue);

private:
  QList<Queue *> m_queues;
};

} // end MoleQueue namespace

#endif // QUEUEMANAGER_H
