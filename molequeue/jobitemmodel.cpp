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

#include "jobitemmodel.h"

#include "job.h"
#include "jobmanager.h"

namespace MoleQueue {

JobItemModel::JobItemModel(JobManager *jobManager, QObject *parentObject)
  : QAbstractItemModel(parentObject), m_jobManager(jobManager)
{
  connect(m_jobManager, SIGNAL(jobAdded(const MoleQueue::Job*)),
          this, SIGNAL(layoutChanged()));
}

int JobItemModel::rowCount(const QModelIndex &modelIndex) const
{
  if (!modelIndex.isValid())
    return m_jobManager->count();
  else
    return 0;
}

int JobItemModel::columnCount(const QModelIndex &) const
{
  return COLUMN_COUNT;
}

QVariant JobItemModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case JOB_TITLE:
      return QVariant("Job Title");
    case QUEUE_NAME:
      return QVariant("Queue");
    case PROGRAM_NAME:
      return QVariant("Program");
    case JOB_STATE:
      return QVariant("Status");
    default:
      return QVariant();
    }
  }

  return QVariant();
}

QVariant JobItemModel::data(const QModelIndex &modelIndex, int role) const
{
  if (!modelIndex.isValid() || modelIndex.column() + 1 > COLUMN_COUNT)
    return QVariant();

  const Job *job = m_jobManager->jobAt(modelIndex.row());
  if (job) {
    if (role == Qt::DisplayRole) {
      switch (modelIndex.column()) {
      case JOB_TITLE:
        return QVariant(job->description());
      case QUEUE_NAME:
        return QVariant(job->queue());
      case PROGRAM_NAME:
        return QVariant(job->program());
      case JOB_STATE:
        return MoleQueue::jobStateToString(job->jobState());
      default:
        return QVariant();
      }
    }
  }
  return QVariant();
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
  if (row >= 0 && row < m_jobManager->count())
    return this->createIndex(row, column);
  else
    return QModelIndex();
}

} // End of namespace
