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

#include "queueitemmodel.h"

namespace MoleQueue {

QueueItemModel::QueueItemModel(QList<Queue *> *queueList,
                               QObject *parentObject)
  : QAbstractItemModel(parentObject)
{
  m_queueList = queueList;
}

QModelIndex QueueItemModel::parent(const QModelIndex &/*modelIndex*/) const
{
  return QModelIndex();
}

int QueueItemModel::rowCount(const QModelIndex &/*modelIndex*/) const
{
  if (m_queueList)
    return m_queueList->size();
  else
    return 0;
}

int QueueItemModel::columnCount(const QModelIndex &/*modelIndex*/) const
{
  return 2;
}

QVariant QueueItemModel::data(const QModelIndex &/*modelIndex*/,
                              int /*role*/) const
{
  return QVariant();
}

bool QueueItemModel::setData(const QModelIndex &/*modelIndex*/,
                             const QVariant &/*value*/,
                             int /*role*/)
{
  return false;
}

Qt::ItemFlags QueueItemModel::flags(const QModelIndex &/*modelIndex*/) const
{
  return Qt::ItemFlags();
}

QModelIndex QueueItemModel::index(int /*row*/, int /*column*/,
                                  const QModelIndex &/*modelIndex*/) const
{
  return QModelIndex();
}

void QueueItemModel::clear()
{
}

void QueueItemModel::add(Queue */*queue*/)
{
}

void QueueItemModel::remove(Queue */*queue*/)
{
}

void QueueItemModel::queuesChanged()
{
}

} // End of namespace
