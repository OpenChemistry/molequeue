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

namespace MoleQueue
{
class JobManager;

class JobItemModel : public QAbstractItemModel
{
  Q_OBJECT

  enum ColumnNames {
    JOB_TITLE = 0,
    QUEUE_NAME,
    PROGRAM_NAME,
    JOB_STATE,

    COLUMN_COUNT // Use to get the total number of columns
  };

public:
  explicit JobItemModel(JobManager *jobManager, QObject *parentObject = 0);

  QModelIndex parent(const QModelIndex &) const {return QModelIndex();}

  int rowCount(const QModelIndex & theModelIndex = QModelIndex()) const;
  int columnCount(const QModelIndex & modelIndex = QModelIndex()) const;

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;

  QVariant data(const QModelIndex & modelIndex, int role = Qt::DisplayRole) const;

  Qt::ItemFlags flags(const QModelIndex & modelIndex) const;

  QModelIndex index(int row, int column,
                    const QModelIndex & modelIndex = QModelIndex()) const;

protected:
  JobManager *m_jobManager;
};

} // End namespace

#endif
