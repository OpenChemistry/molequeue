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

#include "ProgramItemModel.h"

#include "program.h"

namespace MoleQueue {

ProgramItemModel::ProgramItemModel(QList<Program *> *jobList, QObject *parent)
  : QAbstractItemModel(parent)
{
  m_jobList = jobList;
}

QModelIndex ProgramItemModel::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int ProgramItemModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid() && m_jobList)
    return m_jobList->size();
  else
    return 0;
}

int ProgramItemModel::columnCount(const QModelIndex &parent) const
{
  return 4;
}

QVariant ProgramItemModel::headerData(int section, Qt::Orientation orientation,
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
  }
  else {
    return QVariant();
  }
}

QVariant ProgramItemModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || index.column() > 3)
    return QVariant();

  Program *job = static_cast<Program *>(index.internalPointer());
  if (job) {
    if (role == Qt::DisplayRole) {
      switch (index.column()) {
      case 0:
        return QVariant(job->title());
      case 1:
        return QVariant(job->name());
      case 2:
        return QVariant(job->queueName());
      case 3:
        return QVariant(job->statusString());
      default:
        return QVariant();
      }
    }
  }
  return QVariant();
}

bool ProgramItemModel::setData(const QModelIndex &index, const QVariant &value,
                             int role)
{
  return false;
}

Qt::ItemFlags ProgramItemModel::flags(const QModelIndex &index) const
{
  if (index.column() == 0)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  else
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex ProgramItemModel::index(int row, int column,
                                    const QModelIndex &parent) const
{
  if (row >= 0 && row < m_jobList->size())
    return createIndex(row, column, (*m_jobList)[row]);
  else
    return QModelIndex();
}

void ProgramItemModel::clear()
{
}

void ProgramItemModel::add(Program *job)
{
  int row = m_jobList->size();
  beginInsertRows(QModelIndex(), row, row);
  m_jobList->push_back(job);
  endInsertRows();
}

void ProgramItemModel::remove(Program *job)
{
}

void ProgramItemModel::queuesChanged()
{
}

} // End of namespace
