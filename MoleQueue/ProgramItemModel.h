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

#ifndef ProgramItemModel_H
#define ProgramItemModel_H

#include <QtCore/QModelIndex>
#include <QtCore/QList>

namespace MoleQueue {

class Program;

class ProgramItemModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit ProgramItemModel(QList<Program *> *programList, QObject *parent = 0);

  QModelIndex parent(const QModelIndex & index) const;
  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;

  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex & index, const QVariant & value,
               int role = Qt::EditRole);
  Qt::ItemFlags flags(const QModelIndex & index) const;

  QModelIndex index(int row, int column,
                    const QModelIndex & parent = QModelIndex()) const;

  void clear();

private:
  QList<Program *> *m_jobList;

public slots:
  void add(Program *program);
  void remove(Program *program);

  void queuesChanged();
};

} // End namespace

#endif
