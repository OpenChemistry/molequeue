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

#include "advancedfilterdialog.h"
#include "ui_advancedfilterdialog.h"

#include "jobtableproxymodel.h"

namespace MoleQueue {

AdvancedFilterDialog::AdvancedFilterDialog(JobTableProxyModel *model,
                                           QWidget *par) :
  QDialog(par),
  ui(new Ui::AdvancedFilterDialog),
  m_proxyModel(model)
{
  ui->setupUi(this);

  ui->filterStatusNew->setChecked(m_proxyModel->showStatusNew());
  ui->filterStatusSubmitted->setChecked(m_proxyModel->showStatusSubmitted());
  ui->filterStatusQueued->setChecked(m_proxyModel->showStatusQueued());
  ui->filterStatusRunning->setChecked(m_proxyModel->showStatusRunning());
  ui->filterStatusFinished->setChecked(m_proxyModel->showStatusFinished());
  ui->filterStatusKilled->setChecked(m_proxyModel->showStatusKilled());
  ui->filterStatusError->setChecked(m_proxyModel->showStatusError());
  ui->filterShowHidden->setChecked(m_proxyModel->showHiddenJobs());

  connect(ui->filterStatusAll, SIGNAL(clicked()),
          this, SLOT(selectAllStatuses()));
  connect(ui->filterStatusNone, SIGNAL(clicked()),
          this, SLOT(selectNoStatuses()));

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
}

AdvancedFilterDialog::~AdvancedFilterDialog()
{
  delete ui;
}

void AdvancedFilterDialog::updateFilters()
{
  m_proxyModel->setShowStatusNew(ui->filterStatusNew->isChecked());
  m_proxyModel->setShowStatusSubmitted(ui->filterStatusSubmitted->isChecked());
  m_proxyModel->setShowStatusQueued(ui->filterStatusQueued->isChecked());
  m_proxyModel->setShowStatusRunning(ui->filterStatusRunning->isChecked());
  m_proxyModel->setShowStatusFinished(ui->filterStatusFinished->isChecked());
  m_proxyModel->setShowStatusKilled(ui->filterStatusKilled->isChecked());
  m_proxyModel->setShowStatusError(ui->filterStatusError->isChecked());
  m_proxyModel->setShowHiddenJobs(ui->filterShowHidden->isChecked());
}

void AdvancedFilterDialog::selectAllStatuses()
{
  ui->filterStatusNew->setChecked(true);
  ui->filterStatusSubmitted->setChecked(true);
  ui->filterStatusQueued->setChecked(true);
  ui->filterStatusRunning->setChecked(true);
  ui->filterStatusFinished->setChecked(true);
  ui->filterStatusKilled->setChecked(true);
  ui->filterStatusError->setChecked(true);
}

void AdvancedFilterDialog::selectNoStatuses()
{
  ui->filterStatusNew->setChecked(false);
  ui->filterStatusSubmitted->setChecked(false);
  ui->filterStatusQueued->setChecked(false);
  ui->filterStatusRunning->setChecked(false);
  ui->filterStatusFinished->setChecked(false);
  ui->filterStatusKilled->setChecked(false);
  ui->filterStatusError->setChecked(false);
}

} // namespace MoleQueue
