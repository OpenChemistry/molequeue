#include "QueueItemModel.h"

namespace MoleQueue {

QueueItemModel::QueueItemModel(QueueList *queueList, QObject *parent)
  : QAbstractItemModel(parent)
{
  m_queueList = queueList;
}

QModelIndex QueueItemModel::parent(const QModelIndex &index) const
{
}

int QueueItemModel::rowCount(const QModelIndex &parent) const
{

}

int QueueItemModel::columnCount(const QModelIndex &parent) const
{
}

QVariant QueueItemModel::data(const QModelIndex &index, int role) const
{
}

bool QueueItemModel::setData(const QModelIndex &index, const QVariant &value,
                             int role)
{
}

Qt::ItemFlags QueueItemModel::flags(const QModelIndex &index) const
{
}

QModelIndex QueueItemModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
}

void QueueItemModel::clear()
{
}

void QueueItemModel::add(Queue *queue)
{
}

void QueueItemModel::remove(Queue *queue)
{
}

void QueueItemModel::queuesChanged()
{
}

} // End of namespace
