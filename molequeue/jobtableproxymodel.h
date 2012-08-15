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

#ifndef MOLEQUEUE_JOBTABLEPROXYMODEL_H
#define MOLEQUEUE_JOBTABLEPROXYMODEL_H

#include <QtGui/QSortFilterProxyModel>

namespace MoleQueue {

/// @brief Filtering item model for the JobTableWidget job list.
class JobTableProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  explicit JobTableProxyModel(QObject *parent_ = 0);

signals:

public slots:
  void setFilterString(const QString &str);

  void setShowStatusNew(bool show);
  void setShowStatusSubmitted(bool show);
  void setShowStatusQueued(bool show);
  void setShowStatusRunning(bool show);
  void setShowStatusFinished(bool show);
  void setShowStatusKilled(bool show);
  void setShowStatusError(bool show);

  void setShowHiddenJobs(bool show);

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
  QString m_filterString;
  bool m_showStatusNew;
  bool m_showStatusSubmitted;
  bool m_showStatusQueued;
  bool m_showStatusRunning;
  bool m_showStatusFinished;
  bool m_showStatusKilled;
  bool m_showStatusError;

  bool m_showHiddenJobs;

};

} // namespace MoleQueue

#endif // MOLEQUEUE_JOBTABLEPROXYMODEL_H
