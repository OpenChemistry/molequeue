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
  m_proxyModel(new JobTableProxyModel (this))
{
  ui->setupUi(this);

  ui->table->setModel(m_proxyModel);
  ui->table->setSortingEnabled(true);

  connect(ui->filterStatusAll, SIGNAL(clicked()),
          this, SLOT(selectAllStatuses()));
  connect(ui->filterStatusNone, SIGNAL(clicked()),
          this, SLOT(selectNoStatuses()));

  connect(ui->push_clearFinished, SIGNAL(clicked()),
          this, SLOT(clearFinishedJobs()));
  connect(ui->filterEdit, SIGNAL(textChanged(QString)),
          this, SLOT(updateFilters()));
  connect(ui->filterStatusNew, SIGNAL(toggled(bool)),
          this, SLOT(updateFilters()));
  connect(ui->filterStatusSubmitted, SIGNAL(toggled(bool)),
          this, SLOT(updateFilters()));
  connect(ui->filterStatusQueued, SIGNAL(toggled(bool)),
          this, SLOT(updateFilters()));
  connect(ui->filterStatusRunning, SIGNAL(toggled(bool)),
          this, SLOT(updateFilters()));
  connect(ui->filterStatusFinished, SIGNAL(toggled(bool)),
          this, SLOT(updateFilters()));
  connect(ui->filterStatusKilled, SIGNAL(toggled(bool)),
          this, SLOT(updateFilters()));
  connect(ui->filterStatusError, SIGNAL(toggled(bool)),
          this, SLOT(updateFilters()));
  connect(ui->filterShowHidden, SIGNAL(toggled(bool)),
          this, SLOT(updateFilters()));

  restoreUiState();
}

JobTableWidget::~JobTableWidget()
{
  saveUiState();
  delete ui;
}


void JobTableWidget::setJobManager(MoleQueue::JobManager *jobMan)
{
  if (jobMan == m_jobManager)
    return;

  m_jobManager = jobMan;
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

void JobTableWidget::saveUiState()
{
  QSettings settings;
  settings.beginGroup("jobTable");
  settings.beginGroup("filter");
  settings.setValue("filterString", ui->filterEdit->text());
  settings.setValue("showMore", ui->filterMore->isChecked());

  settings.beginGroup("status");
  settings.setValue("new", ui->filterStatusNew->isChecked());
  settings.setValue("submitted", ui->filterStatusSubmitted->isChecked());
  settings.setValue("queued", ui->filterStatusQueued->isChecked());
  settings.setValue("running", ui->filterStatusRunning->isChecked());
  settings.setValue("finished", ui->filterStatusFinished->isChecked());
  settings.setValue("killed", ui->filterStatusKilled->isChecked());
  settings.setValue("error", ui->filterStatusError->isChecked());
  settings.endGroup(); // status

  settings.setValue("showHidden", ui->filterShowHidden->isChecked());

  settings.endGroup(); // filter
  settings.endGroup(); // jobTable
}

void JobTableWidget::restoreUiState()
{
  blockFilterUiSignals(true);
  QSettings settings;
  settings.beginGroup("jobTable");
  settings.beginGroup("filter");
  ui->filterEdit->setText(settings.value("filterString").toString());
  ui->filterMore->setChecked(settings.value("showMore", false).toBool());

  settings.beginGroup("status");
  ui->filterStatusNew->setChecked(settings.value("new", true).toBool());
  ui->filterStatusSubmitted->setChecked(
        settings.value("submitted", true).toBool());
  ui->filterStatusQueued->setChecked(
        settings.value("queued", true).toBool());
  ui->filterStatusRunning->setChecked(
        settings.value("running", true).toBool());
  ui->filterStatusFinished->setChecked(
        settings.value("finished", true).toBool());
  ui->filterStatusKilled->setChecked(
        settings.value("killed", true).toBool());
  ui->filterStatusError->setChecked(
        settings.value("error", true).toBool());
  settings.endGroup(); // status

  ui->filterShowHidden->setChecked(
        settings.value("showHidden", true).toBool());

  settings.endGroup(); // filter
  settings.endGroup(); // jobTable
  blockFilterUiSignals(false);
  updateFilters();
}

void JobTableWidget::blockFilterUiSignals(bool block)
{
  ui->filterEdit->blockSignals(block);
  ui->filterStatusNew->blockSignals(block);
  ui->filterStatusSubmitted->blockSignals(block);
  ui->filterStatusQueued->blockSignals(block);
  ui->filterStatusRunning->blockSignals(block);
  ui->filterStatusFinished->blockSignals(block);
  ui->filterStatusKilled->blockSignals(block);
  ui->filterStatusError->blockSignals(block);
  ui->filterShowHidden->blockSignals(block);
}

void JobTableWidget::updateFilters()
{
  saveUiState();
  m_proxyModel->setFilterString(ui->filterEdit->text());
  m_proxyModel->setShowStatusNew(ui->filterStatusNew->isChecked());
  m_proxyModel->setShowStatusSubmitted(ui->filterStatusSubmitted->isChecked());
  m_proxyModel->setShowStatusQueued(ui->filterStatusQueued->isChecked());
  m_proxyModel->setShowStatusRunning(ui->filterStatusRunning->isChecked());
  m_proxyModel->setShowStatusFinished(ui->filterStatusFinished->isChecked());
  m_proxyModel->setShowStatusKilled(ui->filterStatusKilled->isChecked());
  m_proxyModel->setShowStatusError(ui->filterStatusError->isChecked());
  m_proxyModel->setShowHiddenJobs(ui->filterShowHidden->isChecked());
}

void JobTableWidget::selectAllStatuses()
{
  blockFilterUiSignals(true);
  ui->filterStatusNew->setChecked(true);
  ui->filterStatusSubmitted->setChecked(true);
  ui->filterStatusQueued->setChecked(true);
  ui->filterStatusRunning->setChecked(true);
  ui->filterStatusFinished->setChecked(true);
  ui->filterStatusKilled->setChecked(true);
  ui->filterStatusError->setChecked(true);
  blockFilterUiSignals(false);
  updateFilters();
}

void JobTableWidget::selectNoStatuses()
{
  blockFilterUiSignals(true);
  ui->filterStatusNew->setChecked(false);
  ui->filterStatusSubmitted->setChecked(false);
  ui->filterStatusQueued->setChecked(false);
  ui->filterStatusRunning->setChecked(false);
  ui->filterStatusFinished->setChecked(false);
  ui->filterStatusKilled->setChecked(false);
  ui->filterStatusError->setChecked(false);
  blockFilterUiSignals(false);
  updateFilters();
}

} // end namespace MoleQueue
