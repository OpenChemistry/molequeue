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

#include "openwithmanagerdialog.h"
#include "ui_openwithmanagerdialog.h"

#include "actionfactorymanager.h"
#include "jobactionfactories/programmableopenwithactionfactory.h"
#include "openwithexecutablemodel.h"
#include "openwithpatternmodel.h"
#include "patterntypedelegate.h"

#include <QtGui/QDataWidgetMapper>
#include <QtGui/QHeaderView>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QKeyEvent>
#include <QtGui/QPalette>
#include <QtGui/QStringListModel>

namespace MoleQueue
{

OpenWithManagerDialog::OpenWithManagerDialog(QWidget *parentObject) :
  QDialog(parentObject),
  ui(new Ui::OpenWithManagerDialog),
  m_execModel(new OpenWithExecutableModel (this)),
  m_patternModel(new OpenWithPatternModel (this)),
  m_patternMapper(new QDataWidgetMapper (this)),
  m_execMapper(new QDataWidgetMapper (this)),
  m_patternTypeDelegate(new PatternTypeDelegate (this))
{
  ui->setupUi(this);

  // Get the programmableopenwithactionfactories from the manager and copy them
  // into the cached m_factories.
  ActionFactoryManager *manager = ActionFactoryManager::getInstance();
  m_origFactories =
      manager->getFactories(JobActionFactory::ProgrammableOpenWith);
  foreach (JobActionFactory *factory,m_origFactories) {
    m_factories << ProgrammableOpenWithActionFactory(
                     *static_cast<ProgrammableOpenWithActionFactory*>(factory));
  }

  m_execModel->setFactories(&m_factories);
  ui->group_pattern->setDisabled(true);

  ui->list_exec->setModel(m_execModel);

  ui->table_pattern->setModel(m_patternModel);
  ui->table_pattern->setItemDelegate(m_patternTypeDelegate);

  ui->combo_match->setModel(m_patternTypeDelegate->patternTypeModel());

  m_execMapper->setModel(m_execModel);
  m_execMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
  m_execMapper->addMapping(ui->edit_exec, 0);

  m_patternMapper->setModel(m_patternModel);
  m_patternMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  m_patternMapper->setItemDelegate(m_patternTypeDelegate);
  m_patternMapper->addMapping(ui->edit_pattern,
                              OpenWithPatternModel::PatternCol);
  m_patternMapper->addMapping(ui->combo_match,
                              OpenWithPatternModel::PatternTypeCol);
  m_patternMapper->addMapping(ui->check_caseSensitive,
                              OpenWithPatternModel::CaseSensitivityCol);

  // Executable GUI:
  connect(ui->push_addExec, SIGNAL(clicked()),
          this, SLOT(addExecutable()));
  connect(ui->push_removeExec, SIGNAL(clicked()),
          this, SLOT(removeExecutable()));
  connect(ui->list_exec->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(executableSelectionChanged()));
  connect(m_execModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
          this, SLOT(executableDimensionsChanged()));
  connect(m_execModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          this, SLOT(executableDimensionsChanged()));
  connect(m_execModel, SIGNAL(modelReset()),
          this, SLOT(executableDimensionsChanged()));

  // Pattern GUI:
  connect(ui->push_addPattern, SIGNAL(clicked()),
          this, SLOT(addPattern()));
  connect(ui->push_removePattern, SIGNAL(clicked()),
          this, SLOT(removePattern()));
  connect(ui->table_pattern->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(patternSelectionChanged()));
  connect(ui->push_applyPattern, SIGNAL(clicked()),
          m_patternMapper, SLOT(submit()));
  connect(ui->push_revertPattern, SIGNAL(clicked()),
          m_patternMapper, SLOT(revert()));
  connect(m_patternModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
          this, SLOT(patternDimensionsChanged()));
  connect(m_patternModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          this, SLOT(patternDimensionsChanged()));
  connect(m_patternModel, SIGNAL(modelReset()),
          this, SLOT(patternDimensionsChanged()));

  // Test updates:
  connect(ui->edit_test, SIGNAL(textChanged(QString)),
          this, SLOT(checkTestText()));
  connect(m_patternModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(checkTestText()));
  connect(m_patternModel, SIGNAL(layoutChanged()),
          this, SLOT(checkTestText()));

  // Initialize GUI state:
  this->executableDimensionsChanged();
  this->patternDimensionsChanged();
}

OpenWithManagerDialog::~OpenWithManagerDialog()
{
  delete ui;
}

void OpenWithManagerDialog::accept()
{
  // Delete the original factories from the manager and replace them with our
  // new ones
  ActionFactoryManager *manager = ActionFactoryManager::getInstance();
  foreach (JobActionFactory *factory, m_origFactories)
    manager->removeFactory(factory);
  foreach (ProgrammableOpenWithActionFactory factory, m_factories)
    manager->addFactory(new ProgrammableOpenWithActionFactory(factory));

  this->QDialog::accept();
}

void OpenWithManagerDialog::reject()
{
  this->QDialog::reject();
}

void OpenWithManagerDialog::addExecutable()
{
  QModelIndexList sel = this->selectedExecutableIndices();
  int index = -1;
  if (sel.size() == 1)
    index = sel.first().row();

  if (index+1 > m_execModel->rowCount(QModelIndex()) || index < 0)
    index = m_execModel->rowCount(QModelIndex());

  m_execModel->insertRow(index);

  ui->list_exec->selectionModel()->select(
        m_execModel->index(index, 0),
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void OpenWithManagerDialog::removeExecutable()
{
  QModelIndexList sel = this->selectedExecutableIndices();
  if (sel.size() != 1)
    return;

  int index = sel.first().row();

  if (index+1 > m_execModel->rowCount(QModelIndex()) || index < 0)
    return;

  m_execModel->removeRow(index);
}

void OpenWithManagerDialog::executableSelectionChanged()
{
  // Get selected executable
  QModelIndexList sel = this->selectedExecutableIndices();
  int index = -1;
  if (sel.size() == 1)
    index = sel.first().row();

  // If valid, set the regexp list
  if (index >= 0 && index < m_execModel->rowCount(QModelIndex())) {
    ui->group_pattern->setEnabled(true);
    m_patternModel->setRegExps(&m_factories[index].recognizedFilePatterns());
    m_patternMapper->toFirst();
  }
  // otherwise, clear the regexp list and disable the pattern GUI
  else {
    m_patternModel->setRegExps(NULL);
    ui->group_pattern->setEnabled(false);
  }

  // Update the execMapper
  if (sel.size())
    m_execMapper->setCurrentIndex(sel.first().row());
}

void OpenWithManagerDialog::executableDimensionsChanged()
{
  this->setExecutableGuiEnabled(m_execModel->rowCount(QModelIndex()) != 0);
}

void OpenWithManagerDialog::setExecutableGuiEnabled(bool enable)
{
  ui->edit_exec->setEnabled(enable);
  ui->label_exec->setEnabled(enable);
}

void OpenWithManagerDialog::addPattern()
{
  QModelIndexList sel = this->selectedPatternIndices();
  int index = -1;
  // Should hold a single row:
  if (sel.size() == OpenWithPatternModel::COLUMN_COUNT)
    index = sel.first().row();

  if (index+1 > m_patternModel->rowCount(QModelIndex()) || index < 0)
    index = m_patternModel->rowCount(QModelIndex());

  m_patternModel->insertRow(index);

  ui->table_pattern->selectionModel()->select(
        m_patternModel->index(index, 0, QModelIndex()),
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void OpenWithManagerDialog::removePattern()
{
  QModelIndexList sel = this->selectedPatternIndices();
  // Should hold a single row:
  if (sel.size() != OpenWithPatternModel::COLUMN_COUNT)
    return;

  int index = sel.first().row();

  if (index < 0 || index >= m_patternModel->rowCount(QModelIndex()))
    return;

  m_patternModel->removeRow(index);
}

void OpenWithManagerDialog::patternSelectionChanged()
{
  QModelIndexList sel = this->selectedPatternIndices();

  if (sel.size())
    m_patternMapper->setCurrentIndex(sel.first().row());
}

void OpenWithManagerDialog::patternDimensionsChanged()
{
  this->setPatternGuiEnabled(m_patternModel->rowCount(QModelIndex()) != 0);
  ui->table_pattern->horizontalHeader()->setResizeMode(
        OpenWithPatternModel::PatternCol, QHeaderView::Stretch);
}

void OpenWithManagerDialog::setPatternGuiEnabled(bool enable)
{
  ui->label_pattern->setEnabled(enable);
  ui->edit_pattern->setEnabled(enable);
  ui->combo_match->setEnabled(enable);
  ui->check_caseSensitive->setEnabled(enable);
  ui->push_applyPattern->setEnabled(enable);
  ui->push_revertPattern->setEnabled(enable);
}

void OpenWithManagerDialog::checkTestText()
{
  ProgrammableOpenWithActionFactory *factory = this->selectedFactory();

  if (!factory) {
    this->testTextNoMatch();
    return;
  }

  const QString testText = ui->edit_test->text();
  foreach (const QRegExp &regexp, factory->recognizedFilePatterns()) {
    if (regexp.indexIn(testText) >= 0) {
      this->testTextMatch();
      return;
    }
  }
  this->testTextNoMatch();
  return;
}

void OpenWithManagerDialog::testTextMatch()
{
  QPalette pal;
  pal.setColor(QPalette::Text, Qt::darkGreen);
  ui->edit_test->setPalette(pal);
}

void OpenWithManagerDialog::testTextNoMatch()
{
  QPalette pal;
  pal.setColor(QPalette::Text, Qt::red);
  ui->edit_test->setPalette(pal);
}

QModelIndexList OpenWithManagerDialog::selectedExecutableIndices() const
{
  return ui->list_exec->selectionModel()->selectedIndexes();
}

QModelIndexList OpenWithManagerDialog::selectedPatternIndices() const
{
  return ui->table_pattern->selectionModel()->selectedIndexes();
}

ProgrammableOpenWithActionFactory *OpenWithManagerDialog::selectedFactory()
{
  QModelIndexList sel = this->selectedExecutableIndices();
  if (sel.size() != 1)
    return NULL;

  int index = sel.first().row();

  if (index < 0 || index >= m_factories.size())
    return NULL;

  return &m_factories[index];
}

QRegExp *OpenWithManagerDialog::selectedRegExp()
{
  QModelIndexList sel = this->selectedPatternIndices();
  // Should hold a single row:
  if (sel.size() != OpenWithPatternModel::COLUMN_COUNT)
    return NULL;

  int index = sel.first().row();

  ProgrammableOpenWithActionFactory *factory = selectedFactory();

  if (!factory)
    return NULL;

  if (index < 0 || index >= factory->recognizedFilePatterns().size())
    return NULL;

  return &factory->recognizedFilePatterns()[index];
}

void OpenWithManagerDialog::keyPressEvent(QKeyEvent *ev)
{
  // Disable forwarding of enter and return to Ok/Cancel buttons. Too easy to
  // accidentally close the dialog while modifying line edits.
  switch (ev->key()) {
  case Qt::Key_Return:
  case Qt::Key_Enter:
    break;
  default:
    QDialog::keyPressEvent(ev);
  }
}

} // end namespace MoleQueue
