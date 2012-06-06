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


void JobTableWidget::setJobManager(MoleQueue::JobManager *jobManager)
{
  m_jobItemModel->setJobManager(jobManager);
}

QList<const Job *> JobTableWidget::getSelectedJobs()
{
  QList<const Job *> result;

  if (!m_jobItemModel->jobManager()) {
    return result;
  }

  QList<const Job *> allJobs = m_jobItemModel->jobManager()->jobs();

  foreach (int row, ui->table->getSelectedRows())
    result << allJobs.at(row);

  return result;
}

void JobTableWidget::clearFinishedJobs()
{
  JobManager *jobManager = m_jobItemModel->jobManager();
  if (!jobManager)
    return;

  QList<const Job*> finishedJobs =
      jobManager->jobsWithJobState(MoleQueue::Finished);

  QMessageBox::StandardButton confirm =
      QMessageBox::question(this, tr("Really remove jobs?"),
                            tr("Are you sure you would like to remove %n "
                               "finished job(s)? This will not delete any input"
                               " or output files.", "", finishedJobs.size()),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (confirm != QMessageBox::Yes)
    return;

  jobManager->removeJobs(finishedJobs);
}

} // end namespace MoleQueue
