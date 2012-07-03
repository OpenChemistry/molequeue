#ifndef MOLEQUEUE_PATTERNTYPEDELEGATE_H
#define MOLEQUEUE_PATTERNTYPEDELEGATE_H

#include <QtGui/QItemDelegate>

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
