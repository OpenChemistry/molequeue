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

#ifndef MOLEQUEUE_OPENWITHEXECUTABLEMODEL_H
#define MOLEQUEUE_OPENWITHEXECUTABLEMODEL_H

#include <QtCore/QAbstractItemModel>

namespace MoleQueue
{
class OpenWithActionFactory;

/// MVC item model for OpenWithActionFactory configurations.
class OpenWithExecutableModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit OpenWithExecutableModel(QObject *parentObject = 0);

  int rowCount(const QModelIndex &p = QModelIndex()) const;
  int columnCount(const QModelIndex &p = QModelIndex()) const;
  QVariant data(const QModelIndex &ind, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  virtual bool insertRows(int row, int count, const QModelIndex &parent);
  virtual bool removeRows(int row, int count, const QModelIndex &parent);

  bool setData(const QModelIndex &ind, const QVariant &value, int role);

  Qt::ItemFlags flags(const QModelIndex &index) const;

  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &child) const;

public slots:
  void setFactories(QList<OpenWithActionFactory> *factories);

protected:
  QList<OpenWithActionFactory> *m_factories;
};

} // namespace MoleQueue

#endif // MOLEQUEUE_OPENWITHEXECUTABLEMODEL_H
