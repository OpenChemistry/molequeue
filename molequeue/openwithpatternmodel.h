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

#ifndef OPENWITHPATTERNMODEL_H
#define OPENWITHPATTERNMODEL_H

#include <QtCore/QAbstractItemModel>

namespace MoleQueue
{
class ProgrammableOpenWithActionFactory;

class OpenWithPatternModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit OpenWithPatternModel(QObject *parentObject = 0);

  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &ind, int role) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  bool insertRows(int row, int count, const QModelIndex &parent);
  bool removeRows(int row, int count, const QModelIndex &parent);

  Qt::ItemFlags flags(const QModelIndex &index) const;

  QModelIndex index(int row, int column, const QModelIndex &) const
  {
    return this->createIndex(row, column);
  }

  QModelIndex parent(const QModelIndex &) const
  {
    return QModelIndex();
  }

  enum ColumnType {
    PatternCol,
    PatternTypeCol,
    CaseSensitivityCol,

    COLUMN_COUNT
  };

  enum PatternType {
    WildCard = 0,
    RegExp,

    PATTERNTYPE_COUNT
  };

  enum CustomRoleType {
    ComboIndexRole = Qt::UserRole
  };

public slots:
  void setRegExps(QList<QRegExp> *regexps);

protected:
  bool indexIsValid(const QModelIndex &ind) const;
  QList<QRegExp> *m_regexps;
};

} // end namespace MoleQueue

#endif // OPENWITHPATTERNMODEL_H
