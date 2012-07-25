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
  m_jobManager(NULL)
{
  ui->setupUi(this);

  connect(ui->push_clearFinished, SIGNAL(clicked()),
          this, SLOT(clearFinishedJobs()));
}

JobTableWidget::~JobTableWidget()
{
  delete ui;
}


void JobTableWidget::setJobManager(MoleQueue::JobManager *jobMan)
{
  if (jobMan == m_jobManager)
    return;

  m_jobManager = jobMan;
  ui->table->setModel(jobMan->itemModel());
  ui->table->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
}

QList<Job> JobTableWidget::getSelectedJobs()
{
  QList<Job> result;

  if (!m_jobManager)
    return result;

  foreach (int row, ui->table->getSelectedRows())
    result << m_jobManager->jobAt(row);

  return result;
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

} // end namespace MoleQueue
