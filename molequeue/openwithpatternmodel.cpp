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

#include "openwithpatternmodel.h"

#include "jobactionfactories/programmableopenwithactionfactory.h"

#include <QtCore/QRegExp>

namespace MoleQueue
{

OpenWithPatternModel::OpenWithPatternModel(QObject *parentObject) :
  QAbstractItemModel(parentObject),
  m_regexps(NULL)
{
}

int OpenWithPatternModel::rowCount(const QModelIndex &) const
{
  if (!m_regexps)
    return 0;

  return m_regexps->size();
}

int OpenWithPatternModel::columnCount(const QModelIndex &) const
{
  if (!m_regexps)
    return 0;

  return COLUMN_COUNT;
}

QVariant OpenWithPatternModel::data(const QModelIndex &ind, int role) const
{
  if ((role != Qt::DisplayRole && role != Qt::EditRole &&
       role != Qt::CheckStateRole && role != ComboIndexRole) ||
      !this->indexIsValid(ind))
    return QVariant();

  QRegExp &regexp = (*m_regexps)[ind.row()];

  if (role == Qt::CheckStateRole) {
    if (ind.column() == CaseSensitivityCol) {
      return regexp.caseSensitivity() == Qt::CaseSensitive ? Qt::Checked
                                                           : Qt::Unchecked;
    }
    else
      return QVariant();
  }

  if (role == ComboIndexRole) {
    if (ind.column() == PatternTypeCol) {
      switch (regexp.patternSyntax()) {
      default:
      case QRegExp::Wildcard:
      case QRegExp::WildcardUnix:
      case QRegExp::W3CXmlSchema11:
      case QRegExp::FixedString:
        return static_cast<int>(WildCard);
      case QRegExp::RegExp:
      case QRegExp::RegExp2:
        return static_cast<int>(RegExp);
      }
    }
  }

  switch (static_cast<ColumnType>(ind.column())) {
  case PatternCol:
    return regexp.pattern();
  case PatternTypeCol:
    switch (regexp.patternSyntax()) {
    default:
    case QRegExp::Wildcard:
    case QRegExp::WildcardUnix:
    case QRegExp::W3CXmlSchema11:
    case QRegExp::FixedString:
      return tr("WildCard");
    case QRegExp::RegExp:
    case QRegExp::RegExp2:
      return tr("RegExp");
    }
  case CaseSensitivityCol:
    if (role == Qt::DisplayRole)
      return regexp.caseSensitivity() == Qt::CaseSensitive
          ? tr("Sensitive", "Case sensitive")
          : tr("Insensitive", "Case insensitive");
    else
      return regexp.caseSensitivity() == Qt::CaseSensitive;

  default:
    // Should not happen...
    return QVariant();
  }
}

bool OpenWithPatternModel::setData(const QModelIndex &ind,
                                   const QVariant &value, int role)
{
  if ((role != Qt::EditRole && role != Qt::CheckStateRole) ||
      !this->indexIsValid(ind))
    return false;

  QRegExp &regexp = (*m_regexps)[ind.row()];

  if (role == Qt::CheckStateRole) {
    if (value.canConvert(QVariant::Int)) {
      Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
      if (ind.column() == CaseSensitivityCol) {
        regexp.setCaseSensitivity(state == Qt::Checked ? Qt::CaseSensitive
                                                       : Qt::CaseInsensitive);
        emit dataChanged(ind, ind);
        return true;
      }
    }
    if (value.canConvert(QVariant::Bool)) {
      if (ind.column() == CaseSensitivityCol) {
        regexp.setCaseSensitivity(value.toBool() ? Qt::CaseSensitive
                                                 : Qt::CaseInsensitive);
        emit dataChanged(ind, ind);
        return true;
      }
    }
    else
      return false;
  }

  switch (static_cast<ColumnType>(ind.column())) {
  case PatternCol:
    if (value.canConvert(QVariant::String)) {
      regexp.setPattern(value.toString());
      emit dataChanged(ind, ind);
      return true;
    }
    else
      return false;

  case PatternTypeCol:
    if (value.type() == QVariant::String) {
      QString str = value.toString().simplified();
      if (!str.isEmpty()) {
        QChar firstChar = str.at(0).toLower();
        if (firstChar == QChar('w')) {
          regexp.setPatternSyntax(QRegExp::Wildcard);
          emit dataChanged(ind, ind);
          return true;
        }
        else if (firstChar == QChar('r')) {
          regexp.setPatternSyntax(QRegExp::RegExp);
          emit dataChanged(ind, ind);
          return true;
        }
      }
    }
    else if (value.canConvert(QVariant::Int)) {
      switch (static_cast<OpenWithPatternModel::PatternType>(value.toInt())) {
      case OpenWithPatternModel::WildCard:
        regexp.setPatternSyntax(QRegExp::Wildcard);
        emit dataChanged(ind, ind);
        return true;
      case OpenWithPatternModel::RegExp:
        regexp.setPatternSyntax(QRegExp::RegExp);
        emit dataChanged(ind, ind);
        return true;
      default:
        return false;
      }
    }
    return false;

  case CaseSensitivityCol:
    if (value.canConvert(QVariant::Bool)) {
      regexp.setCaseSensitivity(value.toBool() ? Qt::CaseSensitive
                                               : Qt::CaseInsensitive);
      emit dataChanged(ind, ind);
      return true;
    }
    return false;
  default:
    // Should not happen...
    return false;
  }
}

QVariant OpenWithPatternModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (!m_regexps || orientation != Qt::Horizontal || role != Qt::DisplayRole ||
      section < 0 || section >= COLUMN_COUNT)
    return QVariant();

  switch (static_cast<ColumnType>(section)) {
  case PatternCol:
    return tr("Pattern");
  case PatternTypeCol:
    return tr("Type");
  case CaseSensitivityCol:
    return tr("Case Sensitive");
  default:
    // Should not happen...
    return QVariant();
  }

}

bool OpenWithPatternModel::insertRows(int row, int count,
                                      const QModelIndex &)
{
  if (!m_regexps)
    return false;

  this->beginInsertRows(QModelIndex(), row, row + count - 1);

  for (int i = 0; i < count; ++i) {
    QRegExp newRegExp ("*.*", Qt::CaseInsensitive, QRegExp::Wildcard);
    m_regexps->insert(row, newRegExp);
  }

  this->endInsertRows();
  return true;
}

bool OpenWithPatternModel::removeRows(int row, int count,
                                      const QModelIndex &)
{
  if (!m_regexps)
    return false;

  this->beginRemoveRows(QModelIndex(), row, row + count - 1);

  for (int i = 0; i < count; ++i) {
    m_regexps->removeAt(row);
  }

  this->endRemoveRows();
  return true;
}

Qt::ItemFlags OpenWithPatternModel::flags(const QModelIndex &ind) const
{
  Qt::ItemFlags result;
  if (ind.column() == CaseSensitivityCol)
    result |= Qt::ItemIsUserCheckable;
  else
    result |= Qt::ItemIsEditable;

  result |= static_cast<Qt::ItemFlags>(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
  return result;
}

void OpenWithPatternModel::setRegExps(QList<QRegExp> *regexps)
{
  if (regexps == m_regexps)
    return;

  this->beginResetModel();
  m_regexps = regexps;
  this->endResetModel();
}

bool OpenWithPatternModel::indexIsValid(const QModelIndex &ind) const
{
  return (m_regexps && ind.isValid() &&
          ind.row() >= 0 && ind.row() < m_regexps->size() &&
          ind.column() >= 0 && ind.column() < COLUMN_COUNT);
}

} // end namespace MoleQueue
