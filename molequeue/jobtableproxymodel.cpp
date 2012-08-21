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

#include "jobtableproxymodel.h"

#include "jobitemmodel.h"

#include "job.h"

namespace MoleQueue {

JobTableProxyModel::JobTableProxyModel(QObject *parent_) :
  QSortFilterProxyModel(parent_)
{
  connect(this, SIGNAL(rowsInserted(QModelIndex, int, int)),
          this, SIGNAL(rowCountChanged()));
  connect(this, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          this, SIGNAL(rowCountChanged()));
  connect(this, SIGNAL(modelReset()),
          this, SIGNAL(rowCountChanged()));
  connect(this, SIGNAL(layoutChanged()),
          this, SIGNAL(rowCountChanged()));
}

void JobTableProxyModel::setFilterString(const QString &str)
{
  if (m_filterString == str)
    return;

  m_filterString = str;
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusNew(bool show)
{
  if (m_showStatusNew == show)
    return;

  m_showStatusNew = show;
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusSubmitted(bool show)
{
  if (m_showStatusSubmitted == show)
    return;

  m_showStatusSubmitted = show;
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusQueued(bool show)
{
  if (m_showStatusQueued == show)
    return;

  m_showStatusQueued = show;
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusRunning(bool show)
{
  if (m_showStatusRunning == show)
    return;

  m_showStatusRunning = show;
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusFinished(bool show)
{
  if (m_showStatusFinished == show)
    return;

  m_showStatusFinished = show;
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusKilled(bool show)
{
  if (m_showStatusKilled == show)
    return;

  m_showStatusKilled = show;
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusError(bool show)
{
  if (m_showStatusError == show)
    return;

  m_showStatusError = show;
  invalidateFilter();
}

void JobTableProxyModel::setShowHiddenJobs(bool show)
{
  if (m_showHiddenJobs == show)
    return;

  m_showHiddenJobs = show;
  invalidateFilter();
}

bool JobTableProxyModel::filterAcceptsRow(int sourceRow,
                                          const QModelIndex &sourceParent) const
{
  QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
  Job job = static_cast<JobItemModel*>(sourceModel())->data(
        sourceIndex, JobItemModel::FetchJobRole).value<Job>();

  if (!job.isValid())
    return false;

  if (job.hideFromGui() && !m_showHiddenJobs)
    return false;

  switch (job.jobState()) {
  case Unknown:
  case None:
  case Accepted:
    if (!m_showStatusNew)
      return false;
    break;
  case LocalQueued:
  case RemoteQueued:
    if (!m_showStatusQueued)
      return false;
    break;
  case Submitted:
    if (!m_showStatusSubmitted)
      return false;
    break;
  case RunningLocal:
  case RunningRemote:
    if (!m_showStatusRunning)
      return false;
    break;
  case Finished:
    if (!m_showStatusFinished)
      return false;
    break;
  case Killed:
    if (!m_showStatusKilled)
      return false;
    break;
  case Error:
    if (!m_showStatusError)
      return false;
    break;
  default:
    break;
  }

  if (!m_filterString.isEmpty()) {
    QStringList filterTerms = m_filterString.split(QRegExp("\\s+"),
                                                   QString::SkipEmptyParts);
    foreach (QString fullTerm, filterTerms) {
      bool termMatch = false;
      bool isNegated = false;

      QStringRef term(&fullTerm);
      // terms starting with '-' should not be present
      if (term.startsWith('-')) {
        isNegated = true;
        term = fullTerm.midRef(1);
      }

      for (int i = 0; i < static_cast<int>(sourceModel()->columnCount()); ++i) {
        const QVariant disp = sourceModel()->data(
              sourceModel()->index(sourceRow, i), Qt::DisplayRole);
        if (disp.canConvert(QVariant::String)) {
          if (disp.toString().contains(term, Qt::CaseInsensitive)) {
            termMatch = true;
            break;
          } // end if string matches
        } // end if variant is string
      } // end foreach column

      // If the term matches in a negated search or vice-versa, the row is
      // not shown
      if (termMatch == isNegated)
        return false;

    } // end foreach term


  } // end if filter string exists

  return true;
}

} // namespace MoleQueue
