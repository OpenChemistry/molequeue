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

#include "jobitemmodel.h"

#include "job.h"
#include "queue.h"

namespace MoleQueue {

JobItemModel::JobItemModel(QObject *parentObject)
  : QAbstractItemModel(parentObject)
{
}

void JobItemModel::addQueue(Queue *queue)
{
  if (m_queues.contains(queue))
    return;
  else {
    m_queues.push_back(queue);
    connect(queue, SIGNAL(jobAdded(Job*)), this, SLOT(add(Job*)));
    connect(queue, SIGNAL(jobStateChanged(Job*)), this, SLOT(queuesChanged()));
  }
}

QModelIndex JobItemModel::parent(const QModelIndex &) const
{
  return QModelIndex();
}

int JobItemModel::rowCount(const QModelIndex &modelIndex) const
{
  if (!modelIndex.isValid())
    return m_jobList.size();
  else
    return 0;
}

int JobItemModel::columnCount(const QModelIndex &/*modelIndex*/) const
{
  return 4;
}

QVariant JobItemModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == 0)
      return QVariant("Job Title");
    else if (section == 1)
      return QVariant("Program");
    else if (section == 2)
      return QVariant("Queue");
    else if (section == 3)
      return QVariant("Status");
    else
      return QVariant();
  }
  else {
    return QVariant();
  }
}

QVariant JobItemModel::data(const QModelIndex &modelIndex, int role) const
{
  if (!modelIndex.isValid() || modelIndex.column() > 3)
    return QVariant();

  Job *job = static_cast<Job *>(modelIndex.internalPointer());
  if (job) {
    if (role == Qt::DisplayRole) {
      switch (modelIndex.column()) {
      case 0:
        return QVariant(job->title());
      case 1:
        return QVariant(job->name());
      case 2:
        return QVariant(job->program()->queueName());
      case 3:
        return QVariant(job->statusString());
      default:
        return QVariant();
      }
    }
  }
  return QVariant();
}

bool JobItemModel::setData(const QModelIndex &/*modelIndex*/,
                           const QVariant &/*value*/, int /*role*/)
{
  return false;
}

Qt::ItemFlags JobItemModel::flags(const QModelIndex &modelIndex) const
{
  if (modelIndex.column() == 0)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  else
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex JobItemModel::index(int row, int column,
                                const QModelIndex &/*modelIndex*/) const
{
  if (row >= 0 && row < m_jobList.size())
    return createIndex(row, column, m_jobList[row]);
  else
    return QModelIndex();
}

void JobItemModel::clear()
{
}

void JobItemModel::add(Job *job)
{
  int row = m_jobList.size();
  beginInsertRows(QModelIndex(), row, row);
  m_jobList.push_back(job);
  endInsertRows();
}

void JobItemModel::remove(Job * /*job*/)
{
}

void JobItemModel::queuesChanged()
{
  this->reset();
}

} // End of namespace
