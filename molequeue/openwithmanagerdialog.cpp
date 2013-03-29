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
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QKeyEvent>
#include <QtGui/QPalette>
#include <QtGui/QStringListModel>

#include <QtCore/QProcessEnvironment>
#include <QtCore/QUrl>

namespace MoleQueue
{

OpenWithManagerDialog::OpenWithManagerDialog(QWidget *parentObject) :
  QDialog(parentObject),
  ui(new Ui::OpenWithManagerDialog),
  m_execModel(new OpenWithExecutableModel(this)),
  m_patternModel(new OpenWithPatternModel(this)),
  m_patternMapper(new QDataWidgetMapper(this)),
  m_execMapper(new QDataWidgetMapper(this)),
  m_patternTypeDelegate(new PatternTypeDelegate(this))
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
  ui->groupPattern->setDisabled(true);

  ui->listExec->setModel(m_execModel);

  ui->tablePattern->setModel(m_patternModel);
  ui->tablePattern->setItemDelegate(m_patternTypeDelegate);

  ui->comboMatch->setModel(m_patternTypeDelegate->patternTypeModel());

  m_execMapper->setModel(m_execModel);
  m_execMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
  m_execMapper->addMapping(ui->editExec, 0);

  m_patternMapper->setModel(m_patternModel);
  m_patternMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  m_patternMapper->setItemDelegate(m_patternTypeDelegate);
  m_patternMapper->addMapping(ui->editPattern,
                              OpenWithPatternModel::PatternCol);
  m_patternMapper->addMapping(ui->comboMatch,
                              OpenWithPatternModel::PatternTypeCol);
  m_patternMapper->addMapping(ui->checkCaseSensitive,
                              OpenWithPatternModel::CaseSensitivityCol);

  // Executable GUI:
  connect(ui->pushAddExec, SIGNAL(clicked()),
          this, SLOT(addExecutable()));
  connect(ui->pushRemoveExec, SIGNAL(clicked()),
          this, SLOT(removeExecutable()));
  connect(ui->pushExec, SIGNAL(clicked()), SLOT(browseExecutable()));
  connect(ui->listExec->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(executableSelectionChanged()));
  connect(m_execModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
          this, SLOT(executableDimensionsChanged()));
  connect(m_execModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          this, SLOT(executableDimensionsChanged()));
  connect(m_execModel, SIGNAL(modelReset()),
          this, SLOT(executableDimensionsChanged()));

  // Pattern GUI:
  connect(ui->pushAddPattern, SIGNAL(clicked()),
          this, SLOT(addPattern()));
  connect(ui->pushRemovePattern, SIGNAL(clicked()),
          this, SLOT(removePattern()));
  connect(ui->tablePattern->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(patternSelectionChanged()));
  connect(ui->pushApplyPattern, SIGNAL(clicked()),
          m_patternMapper, SLOT(submit()));
  connect(ui->pushRevertPattern, SIGNAL(clicked()),
          m_patternMapper, SLOT(revert()));
  connect(m_patternModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
          this, SLOT(patternDimensionsChanged()));
  connect(m_patternModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          this, SLOT(patternDimensionsChanged()));
  connect(m_patternModel, SIGNAL(modelReset()),
          this, SLOT(patternDimensionsChanged()));

  // Test updates:
  connect(ui->editTest, SIGNAL(textChanged(QString)),
          this, SLOT(checkTestText()));
  connect(m_patternModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(checkTestText()));
  connect(m_patternModel, SIGNAL(layoutChanged()),
          this, SLOT(checkTestText()));

  // Initialize GUI state:
  executableDimensionsChanged();
  patternDimensionsChanged();
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

  QDialog::accept();
}

void OpenWithManagerDialog::reject()
{
  QDialog::reject();
}

void OpenWithManagerDialog::addExecutable()
{
  QModelIndexList sel = selectedExecutableIndices();
  int index = -1;
  if (sel.size() == 1)
    index = sel.first().row();

  if (index + 1 > m_execModel->rowCount(QModelIndex()) || index < 0)
    index = m_execModel->rowCount(QModelIndex());

  m_execModel->insertRow(index);

  ui->listExec->selectionModel()->select(
        m_execModel->index(index, 0),
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void OpenWithManagerDialog::removeExecutable()
{
  QModelIndexList sel = selectedExecutableIndices();
  if (sel.size() != 1)
    return;

  int index = sel.first().row();

  if (index + 1 > m_execModel->rowCount(QModelIndex()) || index < 0)
    return;

  m_execModel->removeRow(index);
}

void OpenWithManagerDialog::browseExecutable()
{
  QString fileName = ui->editExec->text();
  QFileInfo info(fileName);
  QString initialPath;

  if (!fileName.isEmpty()) {
    // If the executable name is not an absolute path, try to look it up in PATH
    if (info.isAbsolute()) {
      initialPath = info.absolutePath();
    }
    else {
      QString absoluteFilePath = searchPathForExecutable(info.fileName());
      // Found the path; initialize the file dialog to it
      if (!absoluteFilePath.isEmpty()) {
        ui->editExec->setText(absoluteFilePath);
        initialPath = absoluteFilePath;
      }
    }
  }

  // If we didn't find a path above, just use the user's home directory.
  if (initialPath.isEmpty())
    initialPath = QDir::homePath();

  QString newFilePath = QFileDialog::getOpenFileName(
        this, tr("Select executable"), initialPath);

  if (!newFilePath.isEmpty())
    ui->editExec->setText(newFilePath);
}

void OpenWithManagerDialog::executableSelectionChanged()
{
  // Get selected executable
  QModelIndexList sel = selectedExecutableIndices();
  int index = -1;
  if (sel.size() == 1)
    index = sel.first().row();

  // If valid, set the regexp list
  if (index >= 0 && index < m_execModel->rowCount(QModelIndex())) {
    ui->groupPattern->setEnabled(true);
    m_patternModel->setRegExps(&m_factories[index].recognizedFilePatternsRef());
    m_patternMapper->toFirst();
  }
  // otherwise, clear the regexp list and disable the pattern GUI
  else {
    m_patternModel->setRegExps(NULL);
    ui->groupPattern->setEnabled(false);
  }

  // Update the execMapper
  if (sel.size())
    m_execMapper->setCurrentIndex(sel.first().row());
}

void OpenWithManagerDialog::executableDimensionsChanged()
{
  setExecutableGuiEnabled(m_execModel->rowCount(QModelIndex()) != 0);
}

void OpenWithManagerDialog::setExecutableGuiEnabled(bool enable)
{
  ui->editExec->setEnabled(enable);
  ui->labelExec->setEnabled(enable);
}

void OpenWithManagerDialog::addPattern()
{
  QModelIndexList sel = selectedPatternIndices();
  int index = -1;
  // Should hold a single row:
  if (sel.size() == OpenWithPatternModel::COLUMN_COUNT)
    index = sel.first().row();

  if (index+1 > m_patternModel->rowCount(QModelIndex()) || index < 0)
    index = m_patternModel->rowCount(QModelIndex());

  m_patternModel->insertRow(index);

  ui->tablePattern->selectionModel()->select(
        m_patternModel->index(index, 0, QModelIndex()),
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void OpenWithManagerDialog::removePattern()
{
  QModelIndexList sel = selectedPatternIndices();
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
  QModelIndexList sel = selectedPatternIndices();

  if (sel.size())
    m_patternMapper->setCurrentIndex(sel.first().row());
}

void OpenWithManagerDialog::patternDimensionsChanged()
{
  setPatternGuiEnabled(m_patternModel->rowCount(QModelIndex()) != 0);
  ui->tablePattern->horizontalHeader()->setResizeMode(
        OpenWithPatternModel::PatternCol, QHeaderView::Stretch);
}

void OpenWithManagerDialog::setPatternGuiEnabled(bool enable)
{
  ui->labelPattern->setEnabled(enable);
  ui->editPattern->setEnabled(enable);
  ui->comboMatch->setEnabled(enable);
  ui->checkCaseSensitive->setEnabled(enable);
  ui->pushApplyPattern->setEnabled(enable);
  ui->pushRevertPattern->setEnabled(enable);
}

void OpenWithManagerDialog::checkTestText()
{
  ProgrammableOpenWithActionFactory *factory = selectedFactory();

  if (!factory) {
    testTextNoMatch();
    return;
  }

  const QString testText = ui->editTest->text();
  foreach (const QRegExp &regexp, factory->recognizedFilePatterns()) {
    if (regexp.indexIn(testText) >= 0) {
      testTextMatch();
      return;
    }
  }
  testTextNoMatch();
  return;
}

void OpenWithManagerDialog::testTextMatch()
{
  QPalette pal;
  pal.setColor(QPalette::Text, Qt::darkGreen);
  ui->editTest->setPalette(pal);
}

void OpenWithManagerDialog::testTextNoMatch()
{
  QPalette pal;
  pal.setColor(QPalette::Text, Qt::red);
  ui->editTest->setPalette(pal);
}

QString OpenWithManagerDialog::searchPathForExecutable(const QString &exec)
{
  QString result;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if (!env.contains("PATH"))
    return result;

  static QRegExp pathSplitter = QRegExp(
#ifdef Q_OS_WIN32
        ";"
#else // WIN32
        ":"
#endif// WIN32
        );
  QStringList paths =
      env.value("PATH").split(pathSplitter, QString::SkipEmptyParts);

  foreach (const QString &path, paths) {
    QString testPath = QUrl::fromLocalFile(path + "/" + exec).toLocalFile();
    if (!QFile::exists(testPath))
      continue;
    result = testPath;
    break;
  }

  return result;
}

QModelIndexList OpenWithManagerDialog::selectedExecutableIndices() const
{
  return ui->listExec->selectionModel()->selectedIndexes();
}

QModelIndexList OpenWithManagerDialog::selectedPatternIndices() const
{
  return ui->tablePattern->selectionModel()->selectedIndexes();
}

ProgrammableOpenWithActionFactory *OpenWithManagerDialog::selectedFactory()
{
  QModelIndexList sel = selectedExecutableIndices();
  if (sel.size() != 1)
    return NULL;

  int index = sel.first().row();

  if (index < 0 || index >= m_factories.size())
    return NULL;

  return &m_factories[index];
}

QRegExp *OpenWithManagerDialog::selectedRegExp()
{
  QModelIndexList sel = selectedPatternIndices();
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
