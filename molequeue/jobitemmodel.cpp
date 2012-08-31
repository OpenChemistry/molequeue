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

#include <QtCore/QDebug>

namespace MoleQueue {

JobItemModel::JobItemModel(QObject *parentObject)
  : QAbstractItemModel(parentObject),
    m_jobManager(NULL)
{
  connect(this, SIGNAL(rowsInserted(QModelIndex, int, int)),
          this, SIGNAL(rowCountChanged()));
  connect(this, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          this, SIGNAL(rowCountChanged()));
  connect(this, SIGNAL(modelReset()),
          this, SIGNAL(rowCountChanged()));
  connect(this, SIGNAL(layoutChanged()),
          this, SIGNAL(rowCountChanged()));
}

void JobItemModel::setJobManager(JobManager *newJobManager)
{
  if (m_jobManager == newJobManager)
    return;

  if (m_jobManager)
    m_jobManager->disconnect(this);

  m_jobManager = newJobManager;

  connect(newJobManager, SIGNAL(jobUpdated(MoleQueue::Job)),
          this, SLOT(jobUpdated(MoleQueue::Job)));

  reset();
}

int JobItemModel::rowCount(const QModelIndex &modelIndex) const
{
  if (m_jobManager && !modelIndex.isValid())
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
    case MOLEQUEUE_ID:
      return QVariant("#");
    case JOB_TITLE:
      return QVariant("Job Title");
    case NUM_CORES:
      return QVariant("Cores");
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
  if (!m_jobManager || !modelIndex.isValid() ||
      modelIndex.column() + 1 > COLUMN_COUNT)
    return QVariant();

  Job job = m_jobManager->jobAt(modelIndex.row());
  if (job.isValid()) {
    if (role == Qt::DisplayRole) {
      switch (modelIndex.column()) {
      case MOLEQUEUE_ID:
        return QVariant(job.moleQueueId());
      case JOB_TITLE:
        return QVariant(job.description());
      case NUM_CORES:
        return QVariant(job.numberOfCores());
      case QUEUE_NAME: {
        if (job.queueId() != InvalidId)
          return QVariant(QString("%1 (%2)").arg(job.queue())
                          .arg(QString::number(job.queueId())));
        else
          return QVariant(job.queue());
      }
      case PROGRAM_NAME:
        return QVariant(job.program());
      case JOB_STATE:
        return MoleQueue::jobStateToString(job.jobState());
      default:
        return QVariant();
      }
    }
    else if (role == FetchJobRole) {
      return QVariant::fromValue(job);
    }
  }
  return QVariant();
}

bool JobItemModel::removeRows(int row, int count, const QModelIndex &)
{
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  endRemoveRows();
  return true;
}

bool JobItemModel::insertRows(int row, int count, const QModelIndex &)
{
  beginInsertRows(QModelIndex(), row, row + count - 1);
  endInsertRows();
  return true;
}

Qt::ItemFlags JobItemModel::flags(const QModelIndex &) const
{
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex JobItemModel::index(int row, int column,
                                const QModelIndex &/*modelIndex*/) const
{
  if (m_jobManager && row >= 0 && row < m_jobManager->count())
    return createIndex(row, column);
  else
    return QModelIndex();
}

void JobItemModel::jobUpdated(const Job &job)
{
  if (!m_jobManager)
    return;

  int row = m_jobManager->indexOf(job);
  if (row >= 0)
    emit dataChanged(index(row, 0), index(row, COLUMN_COUNT));
}

} // End of namespace
