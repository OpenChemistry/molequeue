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

#include <QtCore/QSettings>

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

  QSettings settings;
  settings.beginGroup("jobTable");
  settings.beginGroup("filter");

  m_filterString = settings.value("filterString").toString();
  m_showHiddenJobs = settings.value("showHidden", true).toBool();

  settings.beginGroup("status");
  m_showStatusNew = settings.value("new", true).toBool();
  m_showStatusSubmitted = settings.value("submitted", true).toBool();
  m_showStatusQueued = settings.value("queued", true).toBool();
  m_showStatusRunning = settings.value("running", true).toBool();
  m_showStatusFinished = settings.value("finished", true).toBool();
  m_showStatusCanceled = settings.value("canceled", true).toBool();
  m_showStatusError = settings.value("error", true).toBool();
  settings.endGroup(); // status

  settings.endGroup(); // filter
  settings.endGroup(); // jobTable
}

JobTableProxyModel::~JobTableProxyModel()
{
  saveState();
}

void JobTableProxyModel::setFilterString(const QString &str)
{
  if (m_filterString == str)
    return;

  m_filterString = str;
  saveState();
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusNew(bool show)
{
  if (m_showStatusNew == show)
    return;

  m_showStatusNew = show;
  saveState();
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusSubmitted(bool show)
{
  if (m_showStatusSubmitted == show)
    return;

  m_showStatusSubmitted = show;
  saveState();
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusQueued(bool show)
{
  if (m_showStatusQueued == show)
    return;

  m_showStatusQueued = show;
  saveState();
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusRunning(bool show)
{
  if (m_showStatusRunning == show)
    return;

  m_showStatusRunning = show;
  saveState();
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusFinished(bool show)
{
  if (m_showStatusFinished == show)
    return;

  m_showStatusFinished = show;
  saveState();
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusCanceled(bool show)
{
  if (m_showStatusCanceled == show)
    return;

  m_showStatusCanceled = show;
  saveState();
  invalidateFilter();
}

void JobTableProxyModel::setShowStatusError(bool show)
{
  if (m_showStatusError == show)
    return;

  m_showStatusError = show;
  saveState();
  invalidateFilter();
}

void JobTableProxyModel::setShowHiddenJobs(bool show)
{
  if (m_showHiddenJobs == show)
    return;

  m_showHiddenJobs = show;
  saveState();
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
  case QueuedLocal:
  case QueuedRemote:
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
  case Canceled:
    if (!m_showStatusCanceled)
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

void JobTableProxyModel::saveState() const
{
  QSettings settings;
  settings.beginGroup("jobTable");
  settings.beginGroup("filter");

  settings.setValue("filterString", m_filterString);
  settings.setValue("showHidden", m_showHiddenJobs);

  settings.beginGroup("status");
  settings.setValue("new", m_showStatusNew);
  settings.setValue("submitted", m_showStatusSubmitted);
  settings.setValue("queued", m_showStatusQueued);
  settings.setValue("running", m_showStatusRunning);
  settings.setValue("finished", m_showStatusFinished);
  settings.setValue("canceled", m_showStatusCanceled);
  settings.setValue("error", m_showStatusError);
  settings.endGroup(); // status

  settings.endGroup(); // filter
  settings.endGroup(); // jobTable
}

} // namespace MoleQueue
