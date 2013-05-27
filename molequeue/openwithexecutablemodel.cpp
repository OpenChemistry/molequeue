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

#include "jobactionfactories/openwithactionfactory.h"

namespace {
enum Column {
  FactoryName = 0,
  Type,
  Target,

  COLUMN_COUNT
};
}

namespace MoleQueue
{

OpenWithExecutableModel::OpenWithExecutableModel(QObject *parentObject) :
  QAbstractItemModel(parentObject),
  m_factories(NULL)
{
}

int OpenWithExecutableModel::rowCount(const QModelIndex &) const
{
  if (!m_factories)
    return 0;

  return m_factories->size();
}

int OpenWithExecutableModel::columnCount(const QModelIndex &) const
{
  return COLUMN_COUNT;
}

QVariant OpenWithExecutableModel::data(const QModelIndex &ind, int role) const
{
  if ((role != Qt::DisplayRole && role != Qt::EditRole)
      || !m_factories
      || !ind.isValid()
      || ind.row() >= m_factories->size() || ind.row() < 0
      || ind.column() >= COLUMN_COUNT || ind.column() < 0) {
    return QVariant();
  }

  switch (static_cast<Column>(ind.column())) {
  case FactoryName:
    return (*m_factories)[ind.row()].name();

  case Type: {
    if (role == Qt::DisplayRole) {
      switch ((*m_factories)[ind.row()].handlerType()) {
      default:
      case OpenWithActionFactory::NoHandler:
        return tr("N/A");
      case OpenWithActionFactory::ExecutableHandler:
        return tr("EXE", "executable abbreviation");
      case OpenWithActionFactory::RpcHandler:
        return tr("RPC", "remote procedure call abbreviation");
      }
    }
    else {
      return (*m_factories)[ind.row()].handlerType();
    }
  }

  case Target: {
    OpenWithActionFactory &f((*m_factories)[ind.row()]);
    switch (f.handlerType()) {
    default:
    case OpenWithActionFactory::NoHandler:
      return QString();
    case OpenWithActionFactory::ExecutableHandler:
      return f.executable();
    case OpenWithActionFactory::RpcHandler:
      return QString("%1@%2").arg(f.rpcMethod(), f.rpcServer());
    }
  }

  default:
    break;
  }

  return QVariant();
}

QVariant OpenWithExecutableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (m_factories
      && role == Qt::DisplayRole
      && orientation == Qt::Horizontal) {
    switch (static_cast<Column>(section)) {
    case FactoryName:
      return tr("Name");
    case Type:
      return tr("Type");
    case Target:
      return tr("Target");
    default:
      break;
    }
  }

  return QVariant();
}

bool OpenWithExecutableModel::insertRows(int row, int count,
                                         const QModelIndex &)
{
  if (!m_factories)
    return false;

  beginInsertRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i) {
    OpenWithActionFactory newFactory;
    newFactory.setName(tr("New%1").arg(count == 1 ? QString("")
                                                  : QString::number(i+1)));
    // Add a default regex that matches everything
    newFactory.filePatternsRef() << QRegExp("*", Qt::CaseInsensitive,
                                            QRegExp::Wildcard);
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
  if (!m_factories
      || !ind.isValid()
      || ind.row() >= m_factories->size() || ind.row() < 0
      || ind.column() >= COLUMN_COUNT || ind.column() < 0
      || role != Qt::EditRole) {
    return false;
  }

  switch (static_cast<Column>(ind.column())) {
  case FactoryName:
    (*m_factories)[ind.row()].setName(value.toString());
    break;
  case Type:
    (*m_factories)[ind.row()].setHandlerType(
          static_cast<OpenWithActionFactory::HandlerType>(value.toInt()));
    break;
  case Target: {
    OpenWithActionFactory &f((*m_factories)[ind.row()]);
    switch (f.handlerType()) {
    default:
    case OpenWithActionFactory::NoHandler:
      break;
    case OpenWithActionFactory::ExecutableHandler:
      f.setExecutable(value.toString());
      break;
    case OpenWithActionFactory::RpcHandler: {
      QString rpcSpec(value.toString());
      int split(rpcSpec.indexOf('@'));
      f.setRpcDetails(rpcSpec.left(split), rpcSpec.mid(split + 1));
      break;
    }
    }
  }

  default:
    break;
  }

  emit dataChanged(ind, ind);
  return true;
}

Qt::ItemFlags OpenWithExecutableModel::flags(const QModelIndex &) const
{
  return static_cast<Qt::ItemFlags>
      (Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
}

QModelIndex OpenWithExecutableModel::index(int row, int column,
                                           const QModelIndex &p) const
{
  if (p.isValid())
    return QModelIndex();

  return createIndex(row, column);
}

QModelIndex OpenWithExecutableModel::parent(const QModelIndex &) const
{
  return QModelIndex();
}

void
OpenWithExecutableModel::setFactories(QList<OpenWithActionFactory> *factories)
{
  if (m_factories == factories)
    return;

  beginResetModel();
  m_factories = factories;
  endResetModel();
}

} // namespace MoleQueue
