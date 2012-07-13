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
}

void JobItemModel::setJobManager(JobManager *newJobManager)
{
  if (m_jobManager == newJobManager)
    return;

  if (m_jobManager)
    m_jobManager->disconnect(this);

  m_jobManager = newJobManager;

  if (m_jobManager) {
    /// @todo these should reset the model, not change the layout
    connect(m_jobManager, SIGNAL(jobAdded(const MoleQueue::Job &)),
            this, SIGNAL(layoutChanged()));
    connect(m_jobManager, SIGNAL(jobRemoved(MoleQueue::IdType)),
            this, SIGNAL(layoutChanged()));
    connect(m_jobManager, SIGNAL(jobStateChanged(const MoleQueue::Job &,
                                                 MoleQueue::JobState,
                                                 MoleQueue::JobState)),
            this, SLOT(jobUpdated(const MoleQueue::Job &)));
    connect(m_jobManager, SIGNAL(jobQueueIdChanged(const MoleQueue::Job &)),
            this, SLOT(jobUpdated(const MoleQueue::Job &)));
  }

  emit layoutChanged();
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
        return QVariant(QString::number(job.moleQueueId()));
      case JOB_TITLE:
        return QVariant(job.description());
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
  if (!m_jobManager)
    return false;

  this->beginRemoveRows(QModelIndex(), row, row + count - 1);

  QList<Job> jobs;
  for (int i = row; i < row + count; ++i)
    jobs << m_jobManager->jobAt(i);

  /// @todo These signals should reset the model, not change the layout
  // Disconnect the layoutChanged signal from the manager and reattach it when
  // finished, otherwise the table will mistakenly pop off the last item in the
  // list. Don't block signals here -- other receivers may need to handle the
  // removal signals.
  disconnect(m_jobManager, SIGNAL(jobRemoved(MoleQueue::IdType)),
             this, SIGNAL(layoutChanged()));
  m_jobManager->removeJobs(jobs);
  connect(m_jobManager, SIGNAL(jobRemoved(MoleQueue::IdType)),
          this, SIGNAL(layoutChanged()));

  this->endRemoveRows();
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
    return this->createIndex(row, column);
  else
    return QModelIndex();
}

void JobItemModel::jobUpdated(const Job &job)
{
  if (!m_jobManager)
    return;

  int row = m_jobManager->indexOf(job);
  if (row >= 0)
    emit dataChanged(this->index(row, 0), this->index(row, COLUMN_COUNT));
}

} // End of namespace
