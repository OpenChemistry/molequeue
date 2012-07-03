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

#include "openwithexecutablemodel.h"

#include "jobactionfactories/programmableopenwithactionfactory.h"

namespace MoleQueue
{

OpenWithExecutableModel::OpenWithExecutableModel(QObject *parentObject) :
  QAbstractListModel(parentObject),
  m_factories(NULL)
{
}

int OpenWithExecutableModel::rowCount(const QModelIndex &) const
{
  if (!m_factories)
    return 0;

  return m_factories->size();
}

QVariant OpenWithExecutableModel::data(const QModelIndex &ind, int role) const
{
  if ((role != Qt::DisplayRole && role != Qt::EditRole) || !m_factories
      || !ind.isValid() || ind.row() + 1 > m_factories->size() || ind.row() < 0)
    return QVariant();

  return (*m_factories)[ind.row()].executableName();
}

QVariant OpenWithExecutableModel::headerData(
    int, Qt::Orientation, int role) const
{
  if (!m_factories || role != Qt::DisplayRole)
    return QVariant();
  return tr("Executable Name");
}

bool OpenWithExecutableModel::insertRows(int row, int count,
                                         const QModelIndex &)
{
  if (!m_factories)
    return false;

  beginInsertRows(QModelIndex(), row, row + count - 1);

  for (int i = 0; i < count; ++i) {
    ProgrammableOpenWithActionFactory newFactory;
    newFactory.setExecutableName(
          tr("New%1").arg(count == 1 ? QString("") : QString::number(i+1)));
    m_factories->insert(row, newFactory);
  }

  endInsertRows();
  return true;
}

bool OpenWithExecutableModel::removeRows(
    int row, int count, const QModelIndex &)
{
  if (!m_factories)
    return false;

  beginRemoveRows(QModelIndex(), row, row + count - 1);

  for (int i = 0; i < count; ++i)
    m_factories->removeAt(row);

  endRemoveRows();
  return true;
}

bool OpenWithExecutableModel::setData(const QModelIndex &ind,
                                      const QVariant &value, int role)
{
  if (!m_factories || !value.canConvert(QVariant::String) || !ind.isValid() ||
      ind.row() + 1 > m_factories->size() || ind.row() < 0 ||
      role != Qt::EditRole)
    return false;

  (*m_factories)[ind.row()].setExecutableName(value.toString());

  emit dataChanged(ind, ind);
  return true;
}

Qt::ItemFlags OpenWithExecutableModel::flags(const QModelIndex &) const
{
  return static_cast<Qt::ItemFlags>
      (Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
}

void OpenWithExecutableModel::setFactories(
    QList<ProgrammableOpenWithActionFactory> *factories)
{
  if (m_factories == factories)
    return;

  beginResetModel();
  m_factories = factories;
  endResetModel();
}

} // namespace MoleQueue
