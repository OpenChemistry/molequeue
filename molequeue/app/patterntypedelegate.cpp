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

#include "patterntypedelegate.h"

#include "openwithpatternmodel.h" // For enums

#include <QtWidgets/QComboBox>
#include <QtCore/QStringListModel>

#include <QtCore/QMetaProperty>

namespace MoleQueue {

PatternTypeDelegate::PatternTypeDelegate(QObject *parentObject) :
  QItemDelegate(parentObject),
  m_patternTypeModel(new QStringListModel (this))
{
  QStringList patternTypes;
  for (int i = 0; i < OpenWithPatternModel::PATTERNTYPE_COUNT; ++i)
    patternTypes.push_back("--Unknown--");
  patternTypes[OpenWithPatternModel::WildCard] = tr("WildCard");
  patternTypes[OpenWithPatternModel::RegExp] = tr("RegExp");
  m_patternTypeModel->setStringList(patternTypes);
}

QWidget *PatternTypeDelegate::createEditor(QWidget *parentWidget,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
  if (index.column() == OpenWithPatternModel::PatternTypeCol) {
    QComboBox *combo = new QComboBox (parentWidget);
    combo->setModel(patternTypeModel());
    return combo;
  }

  return QItemDelegate::createEditor(parentWidget, option, index);
}

void PatternTypeDelegate::updateEditorGeometry(QWidget *editor,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
  if (index.column() == OpenWithPatternModel::PatternTypeCol) {
    editor->setGeometry(option.rect);
    return;
  }
  QItemDelegate::updateEditorGeometry(editor, option, index);
}

void PatternTypeDelegate::setEditorData(QWidget *editor,
                                        const QModelIndex &index) const
{
  if (index.column() == OpenWithPatternModel::PatternTypeCol) {
    if (editor->property("currentIndex").isValid()) {
      QVariant value = index.data(OpenWithPatternModel::ComboIndexRole);
      editor->setProperty("currentIndex", value);
      return;
    }
  }

  QItemDelegate::setEditorData(editor, index);
}

void PatternTypeDelegate::setModelData(QWidget *editor,
                                       QAbstractItemModel *model,
                                       const QModelIndex &index) const
{
  if (index.column() == OpenWithPatternModel::PatternTypeCol) {
    if (editor->property("currentIndex").isValid()) {
      QVariant value = editor->property("currentIndex");
      if (value.isValid()) {
        model->setData(index, value);
        return;
      }
    }
  }
  QItemDelegate::setModelData(editor, model, index);
}

} // namespace MoleQueue
