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

#include "queueprogramitemmodel.h"

#include "program.h"
#include "queue.h"

namespace MoleQueue
{

QueueProgramItemModel::QueueProgramItemModel(Queue *queue,
                                             QObject *parentObject)
  : QAbstractItemModel(parentObject),
    m_queue(queue)
{
  connect(m_queue, SIGNAL(programAdded(QString,MoleQueue::Program*)),
          this, SLOT(callReset()));
}

QModelIndex QueueProgramItemModel::parent(const QModelIndex &) const
{
  return QModelIndex();
}

int QueueProgramItemModel::rowCount(const QModelIndex &modelIndex) const
{
  if (!modelIndex.isValid())
    return m_queue->numPrograms();
  else
    return 0;
}

int QueueProgramItemModel::columnCount(const QModelIndex &/*modelIndex*/) const
{
  return COLUMN_COUNT;
}

QVariant QueueProgramItemModel::headerData(int section,
                                           Qt::Orientation orientation,
                                           int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (static_cast<ColumnNames>(section)) {
    case PROGRAM_NAME:
      return QVariant("Program");
    case COLUMN_COUNT:
    default:
      return QVariant();
    }
  }

  return QVariant();
}

QVariant QueueProgramItemModel::data(const QModelIndex &modelIndex,
                              int role) const
{
  if (!modelIndex.isValid() || modelIndex.column() >= COLUMN_COUNT ||
      modelIndex.row() >= m_queue->numPrograms())
    return QVariant();

  const Program *program = m_queue->programs().at(modelIndex.row());
  if (program) {
    if (role == Qt::DisplayRole) {
      switch (static_cast<ColumnNames>(modelIndex.column())) {
      case PROGRAM_NAME:
        return program->name();
      case COLUMN_COUNT:
      default:
        return QVariant();
      }
    }
  }

  return QVariant();
}

Qt::ItemFlags QueueProgramItemModel::flags(const QModelIndex &) const
{
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool QueueProgramItemModel::removeRows(int row, int count,
                                       const QModelIndex &)
{
  if (!m_queue)
    return false;

  beginRemoveRows(QModelIndex(), row, row + count - 1);

  for (int i = 0; i < count; ++i)
    m_queue->removeProgram(m_queue->programs().at(row));

  endRemoveRows();

  return true;
}

QModelIndex QueueProgramItemModel::index(int row, int column,
                                  const QModelIndex &) const
{
  if (row >= 0 && row < m_queue->numPrograms())
    return createIndex(row, column);
  else
    return QModelIndex();
}

void QueueProgramItemModel::callReset()
{
  reset();
}

} // End of namespace
