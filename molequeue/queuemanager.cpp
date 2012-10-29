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

#include "logger.h"
#include "queue.h"
#include "server.h"
#include "molequeueconfig.h"

// Concrete queue classes
#include "queues/local.h"
#include "queues/pbs.h"
#include "queues/sge.h"
#ifdef MoleQueue_USE_EZHPC_UIT
#include "queues/uit/queueuit.h"
#endif
#include <QtCore/QDebug>
#include <QtCore/QDir>
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

void QueueManager::readSettings()
{
  QDir queueDir(queueConfigDirectory());
  if (!queueDir.exists()) {
    Logger::logWarning(tr("Cannot write queue settings: Queue config "
                          "directory does not exist (%1)")
                     .arg(queueDir.absolutePath()));
    return;
  }

  foreach (const QString &queueFileName,
           queueDir.entryList(QStringList()<<"*.mqq", QDir::Files)) {
    QString absoluteFileName = queueDir.absoluteFilePath(queueFileName);
    QString queueName = QFileInfo(queueFileName).baseName();
    QString queueType = Queue::queueTypeFromFile(absoluteFileName);
    Queue *queue = addQueue(queueName, queueType, this);

    if (queue != NULL) {
      queue->readSettings(absoluteFileName);
    }
    else {
      Logger::logError(tr("Cannot load queue '%1' with type '%2' from '%3'.")
                       .arg(queueName, queueType, absoluteFileName));
    }
  }
}

void QueueManager::writeSettings() const
{
  foreach (const Queue* queue, queues())
    queue->writeSettings();
}

QStringList QueueManager::availableQueues()
{
  QStringList result;
  result << "Local" << "Sun Grid Engine" << "PBS/Torque";
#ifdef MoleQueue_USE_EZHPC_UIT
  result << "ezHPC UIT";
#endif
  return result;
}

bool QueueManager::queueTypeIsValid(const QString &queueType)
{
  return QueueManager::availableQueues().contains(queueType);
}

Queue * QueueManager::addQueue(const QString &queueName,
                               const QString &queueType, bool replace)
{
  if (m_queues.contains(queueName)) {
    if (replace == true)
      m_queues.take(queueName)->deleteLater();
    else
      return NULL;
  }

  Queue * newQueue = NULL;
  if (queueType== "Local")
    newQueue = new QueueLocal(this);
  else if (queueType== "Sun Grid Engine")
    newQueue = new QueueSge(this);
  else if (queueType== "PBS/Torque")
    newQueue = new QueuePbs(this);
#ifdef MoleQueue_USE_EZHPC_UIT
  else if (queueType== "ezHPC UIT")
    newQueue = new QueueUit(this);
#endif
  if (!newQueue)
    return NULL;

  newQueue->setName(queueName);

  m_queues.insert(newQueue->name(), newQueue);
  emit queueAdded(newQueue->name(), newQueue);
  return newQueue;
}

bool QueueManager::removeQueue(const Queue *queue)
{
  return removeQueue(queue->name());
}

bool QueueManager::removeQueue(const QString &name)
{
  if (!m_queues.contains(name))
    return false;

  Queue *queue = m_queues.take(name);

  emit queueRemoved(name, queue);
  QString fileName = queue->stateFileName();
  queue->deleteLater();

  // Remove state file:
  if (!fileName.isEmpty())
    QFile::remove(fileName);

  return true;
}

QueueListType QueueManager::toQueueList() const
{
  QueueListType queueList;
  foreach(const Queue *queue, m_queues)
    queueList.insert(queue->name(), queue->programNames());

  return queueList;
}

void QueueManager::updateRemoteQueues() const
{
  foreach (Queue *queue, m_queues) {
    if (QueueRemote *remote = qobject_cast<QueueRemote*>(queue)) {
      remote->requestQueueUpdate();
    }
  }
}

QString QueueManager::queueConfigDirectory() const
{
  QString result;
  if (m_server) {
    result = m_server->workingDirectoryBase();
  }
  else {
    QSettings settings;
    result = settings.value("workingDirectoryBase").toString();
  }

  if (result.isEmpty()) {
    Logger::logError(tr("Cannot determine queue config directory."));
    return result;
  }

  return result + "/config/queues";
}

} // end MoleQueue namespace
