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

#include "queuemanageritemmodel.h"

#include "queue.h"
#include "queuemanager.h"

namespace MoleQueue
{

QueueManagerItemModel::QueueManagerItemModel(QueueManager *queueManager,
                                             QObject *parentObject) :
  QAbstractItemModel(parentObject),
  m_queueManager(queueManager)
{
  connect(m_queueManager, SIGNAL(queueAdded(QString,MoleQueue::Queue*)),
          this, SIGNAL(layoutChanged()));
  connect(m_queueManager, SIGNAL(queueRemoved(QString,MoleQueue::Queue*)),
          this, SIGNAL(layoutChanged()));
}

int QueueManagerItemModel::rowCount(const QModelIndex &modelIndex) const
{
  if (!modelIndex.isValid())
    return m_queueManager->numQueues();
  else
    return 0;
}

int QueueManagerItemModel::columnCount(const QModelIndex &) const
{
  return COLUMN_COUNT;
}

QVariant QueueManagerItemModel::headerData(int section,
                                           Qt::Orientation orientation,
                                           int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (static_cast<ColumnNames>(section)) {
    case QUEUE_NAME:
      return QVariant("Queue");
    case QUEUE_TYPE:
      return QVariant("Type");
    case NUM_PROGRAMS:
      return QVariant("# Programs");
    case PROGRAM_NAMES:
      return QVariant("Program names");
    case COLUMN_COUNT:
    default:
      return QVariant();
    }
  }

  return QVariant();
}

QVariant QueueManagerItemModel::data(const QModelIndex &modelIndex,
                                     int role) const
{
  if (!modelIndex.isValid() || modelIndex.column() >= COLUMN_COUNT ||
      modelIndex.row() >= m_queueManager->numQueues())
    return QVariant();

  const Queue *queue = m_queueManager->queues().at(modelIndex.row());
  if (queue) {
    if (role == Qt::DisplayRole) {
      switch (static_cast<ColumnNames>(modelIndex.column())) {
      case QUEUE_NAME:
        return queue->name();
      case QUEUE_TYPE:
        return queue->typeName();
      case NUM_PROGRAMS:
        return queue->numPrograms();
      case PROGRAM_NAMES:
        if (queue->numPrograms() != 0)
          return queue->programNames().join(", ");
        else
          return QVariant("None");
      case COLUMN_COUNT:
      default:
        return QVariant();
      }
    }
  }

  return QVariant();
}

Qt::ItemFlags QueueManagerItemModel::flags(const QModelIndex &modelIndex) const
{
  if (modelIndex.column() == 0)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  else
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex QueueManagerItemModel::index(int row, int column,
                                         const QModelIndex &) const
{
  if (row >= 0 && row < m_queueManager->numQueues())
    return this->createIndex(row, column);
  else
    return QModelIndex();
}

} // end namespace MoleQueue
