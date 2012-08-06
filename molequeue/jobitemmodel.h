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

#ifndef JOBITEMMODEL_H
#define JOBITEMMODEL_H

#include <QtCore/QAbstractItemModel>

#include "molequeueglobal.h"

namespace MoleQueue
{
class Job;
class JobManager;

/// @brief Item model for interacting with jobs.
class JobItemModel : public QAbstractItemModel
{
  Q_OBJECT

  enum ColumnNames {
    MOLEQUEUE_ID = 0,
    JOB_TITLE ,
    NUM_CORES,
    QUEUE_NAME,
    PROGRAM_NAME,
    JOB_STATE,

    COLUMN_COUNT // Use to get the total number of columns
  };

public:
  explicit JobItemModel(QObject *parentObject = 0);

  // Used with the data() method to get info.
  enum UserRoles {
    FetchJobRole = Qt::UserRole
  };

  void setJobManager(JobManager *jobManager);
  JobManager *jobManager() const {return m_jobManager;}

  QModelIndex parent(const QModelIndex &) const {return QModelIndex();}

  int rowCount(const QModelIndex & theModelIndex = QModelIndex()) const;
  int columnCount(const QModelIndex & modelIndex = QModelIndex()) const;

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;

  QVariant data(const QModelIndex & modelIndex, int role = Qt::DisplayRole) const;

  /// Remove the rows from the model. Does not modify the underlying data
  /// structure.
  /// @see JobManager::removeJob()
  bool removeRows(int row, int count, const QModelIndex &);

  /// Insert rows into the model. Does not modify the underlying data structure.
  /// @see JobManager::newJob()
  bool insertRows(int row, int count, const QModelIndex &);

  Qt::ItemFlags flags(const QModelIndex & modelIndex) const;

  QModelIndex index(int row, int column,
                    const QModelIndex & modelIndex = QModelIndex()) const;

  friend class MoleQueue::JobManager;

public slots:
  void jobUpdated(const MoleQueue::Job &job);

protected:
  JobManager *m_jobManager;
};

} // End namespace

#endif
