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

#include "jobtablewidget.h"
#include "ui_jobtablewidget.h"

#include "advancedfilterdialog.h"
#include "jobmanager.h"
#include "jobitemmodel.h"
#include "jobtableproxymodel.h"

#include <QtGui/QMessageBox>

#include <QtCore/QSettings>

namespace MoleQueue
{

JobTableWidget::JobTableWidget(QWidget *parentObject) :
  QWidget(parentObject),
  ui(new Ui::JobTableWidget),
  m_jobManager(NULL),
  m_proxyModel(new JobTableProxyModel (this)),
  m_filterDialog(NULL)
{
  ui->setupUi(this);

  connect(m_proxyModel, SIGNAL(rowCountChanged()),
          this, SLOT(modelRowCountChanged()));

  ui->table->setModel(m_proxyModel);
  ui->table->setSortingEnabled(true);

  connect(ui->filterEdit, SIGNAL(textChanged(QString)),
          this, SLOT(updateFilters()));
  connect(ui->filterMore, SIGNAL(clicked()),
          this, SLOT(showAdvancedFilterDialog()));

  ui->filterEdit->setText(m_proxyModel->filterString());
}

JobTableWidget::~JobTableWidget()
{
  delete ui;
}


void JobTableWidget::setJobManager(MoleQueue::JobManager *jobMan)
{
  if (jobMan == m_jobManager)
    return;

  if (m_jobManager) {
    disconnect(m_jobManager->itemModel(), SIGNAL(rowCountChanged()),
               this, SLOT(modelRowCountChanged()));
  }

  m_jobManager = jobMan;
  connect(m_jobManager->itemModel(), SIGNAL(rowCountChanged()),
          this, SLOT(modelRowCountChanged()));
  m_proxyModel->setSourceModel(jobMan->itemModel());
  m_proxyModel->setDynamicSortFilter(true);

  for (int i = 0; i < m_proxyModel->columnCount(); ++i) {
    if (i == JobItemModel::JOB_TITLE) { // stretch description
      ui->table->horizontalHeader()->setResizeMode(i, QHeaderView::Stretch);
    }
    else { // resize to fit others
      ui->table->horizontalHeader()->setResizeMode(
            i, QHeaderView::ResizeToContents);
    }
  }

  modelRowCountChanged();
}

void JobTableWidget::clearFinishedJobs()
{
  if (!m_jobManager)
    return;

  QList<Job> finishedJobs =
      m_jobManager->jobsWithJobState(MoleQueue::Finished);
  finishedJobs.append(m_jobManager->jobsWithJobState(MoleQueue::Killed));

  QMessageBox::StandardButton confirm =
      QMessageBox::question(this, tr("Really remove jobs?"),
                            tr("Are you sure you would like to remove %n "
                               "finished job(s)? This will not delete any input"
                               " or output files.", "", finishedJobs.size()),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (confirm != QMessageBox::Yes)
    return;

  m_jobManager->removeJobs(finishedJobs);
}

void JobTableWidget::showFilterBar(bool visible)
{
  if (visible)
    focusInFilter();
  else
    ui->filterBar->hide();
}

void JobTableWidget::focusInFilter()
{
  if (!ui->filterBar->isVisible())
    ui->filterBar->show();

  ui->filterEdit->setFocus();
  ui->filterEdit->selectAll();
}

void JobTableWidget::showAdvancedFilterDialog()
{
  if (m_filterDialog == NULL) {
    m_filterDialog = new AdvancedFilterDialog(m_proxyModel, this);
  }

  m_filterDialog->show();
  m_filterDialog->raise();
}

void JobTableWidget::updateFilters()
{
  m_proxyModel->setFilterString(ui->filterEdit->text());
}

void JobTableWidget::modelRowCountChanged()
{
  if (m_jobManager)
    emit jobCountsChanged(m_jobManager->itemModel()->rowCount(),
                          m_proxyModel->rowCount());
}

} // end namespace MoleQueue
