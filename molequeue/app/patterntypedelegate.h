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
#ifndef MOLEQUEUE_PATTERNTYPEDELEGATE_H
#define MOLEQUEUE_PATTERNTYPEDELEGATE_H

#include <QtWidgets/QItemDelegate>

class QStringListModel;

namespace MoleQueue {

/// MVC delegate to control ProgrammableOpenWithActionFactory patterns.
class PatternTypeDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  explicit PatternTypeDelegate(QObject *parentObject = 0);

  QWidget *createEditor(QWidget *parentWidget, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const;

  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const;

  QStringListModel * patternTypeModel() const { return m_patternTypeModel; }

protected:
  QStringListModel *m_patternTypeModel;
};

} // namespace MoleQueue

#endif // MOLEQUEUE_PATTERNTYPEDELEGATE_H
