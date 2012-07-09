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

#include <QtGui/QMessageBox>

namespace MoleQueue
{

JobTableWidget::JobTableWidget(QWidget *parentObject) :
  QWidget(parentObject),
  ui(new Ui::JobTableWidget),
  m_jobItemModel(new JobItemModel (this))
{
  ui->setupUi(this);

  ui->table->setModel(m_jobItemModel);
  ui->table->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);

  connect(ui->push_clearFinished, SIGNAL(clicked()),
          this, SLOT(clearFinishedJobs()));
}

JobTableWidget::~JobTableWidget()
{
  delete ui;
}


void JobTableWidget::setJobManager(MoleQueue::JobManager *jobMan)
{
  m_jobItemModel->setJobManager(jobMan);
}

JobManager *JobTableWidget::jobManager() const
{
  return m_jobItemModel->jobManager();
}

QList<Job> JobTableWidget::getSelectedJobs()
{
  QList<Job> result;

  if (!m_jobItemModel->jobManager()) {
    return result;
  }

  foreach (int row, ui->table->getSelectedRows())
    result << m_jobItemModel->jobManager()->jobAt(row);

  return result;
}

void JobTableWidget::clearFinishedJobs()
{
  JobManager *jobMan = m_jobItemModel->jobManager();
  if (!jobMan)
    return;

  QList<Job> finishedJobs =
      jobMan->jobsWithJobState(MoleQueue::Finished);

  QMessageBox::StandardButton confirm =
      QMessageBox::question(this, tr("Really remove jobs?"),
                            tr("Are you sure you would like to remove %n "
                               "finished job(s)? This will not delete any input"
                               " or output files.", "", finishedJobs.size()),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (confirm != QMessageBox::Yes)
    return;

  jobMan->removeJobs(finishedJobs);
}

} // end namespace MoleQueue
