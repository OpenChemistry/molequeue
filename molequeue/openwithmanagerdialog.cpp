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

#include <QtGui/QCompleter>
#include <QtGui/QDataWidgetMapper>
#include <QtGui/QFileDialog>
#include <QtGui/QFileSystemModel>
#include <QtGui/QHeaderView>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QKeyEvent>
#include <QtGui/QMessageBox>
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
  m_patternTypeDelegate(new PatternTypeDelegate(this)),
  m_dirty(false)
{
  // Setup ui:
  ui->setupUi(this);

  // Setup MVC:
  ui->tableExec->setModel(m_execModel);

  ui->tablePattern->setModel(m_patternModel);
  ui->tablePattern->setItemDelegate(m_patternTypeDelegate);

  ui->comboMatch->setModel(m_patternTypeDelegate->patternTypeModel());

  m_execMapper->setModel(m_execModel);
  m_execMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
  m_execMapper->addMapping(ui->editName, 0);
  m_execMapper->addMapping(ui->editExec, 1);

  m_patternMapper->setModel(m_patternModel);
  m_patternMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  m_patternMapper->setItemDelegate(m_patternTypeDelegate);
  m_patternMapper->addMapping(ui->editPattern,
                              OpenWithPatternModel::PatternCol);
  m_patternMapper->addMapping(ui->comboMatch,
                              OpenWithPatternModel::PatternTypeCol);
  m_patternMapper->addMapping(ui->checkCaseSensitive,
                              OpenWithPatternModel::CaseSensitivityCol);

  // Setup executable completion
  QFileSystemModel *fsModel = new QFileSystemModel(this);
  fsModel->setFilter(QDir::Files | QDir::Dirs | QDir::NoDot);
  fsModel->setRootPath(QDir::rootPath());
  QCompleter *fsCompleter = new QCompleter(fsModel, this);
  ui->editExec->setCompleter(fsCompleter);

  // Executable GUI:
  connect(ui->pushAddExec, SIGNAL(clicked()),
          this, SLOT(addExecutable()));
  connect(ui->pushRemoveExec, SIGNAL(clicked()),
          this, SLOT(removeExecutable()));
  connect(ui->pushExec, SIGNAL(clicked()), SLOT(browseExecutable()));
  connect(ui->tableExec->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(executableSelectionChanged()));

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

  // Executable checking
  connect(ui->editExec, SIGNAL(textChanged(QString)), SLOT(testExecutable()));

  // Test updates:
  connect(ui->editTest, SIGNAL(textChanged(QString)),
          this, SLOT(checkTestText()));
  connect(m_patternModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(checkTestText()));
  connect(m_patternModel, SIGNAL(layoutChanged()),
          this, SLOT(checkTestText()));
  connect(ui->tableExec->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(checkTestText()));

  // handle apply button:
  connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
          SLOT(buttonBoxClicked(QAbstractButton*)));

  // Mark dirty when the data changes.
  connect(m_execModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          SLOT(markDirty()));
  connect(m_execModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
          SLOT(markDirty()));
  connect(m_execModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          SLOT(markDirty()));
  connect(m_patternModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          SLOT(markDirty()));
  connect(m_patternModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
          SLOT(markDirty()));
  connect(m_patternModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          SLOT(markDirty()));
}

OpenWithManagerDialog::~OpenWithManagerDialog()
{
  delete ui;
}

void OpenWithManagerDialog::loadFactories()
{
  reset();
  ActionFactoryManager *manager = ActionFactoryManager::getInstance();
  m_origFactories =
      manager->getFactories(JobActionFactory::ProgrammableOpenWith);
  foreach (JobActionFactory *factory,m_origFactories) {
    m_factories << ProgrammableOpenWithActionFactory(
                     *static_cast<ProgrammableOpenWithActionFactory*>(factory));
  }
  m_execModel->setFactories(&m_factories);
}

void OpenWithManagerDialog::reset()
{
  m_factories.clear();
  m_origFactories.clear();
  m_execModel->setFactories(NULL);
  m_patternModel->setRegExps(NULL);
  setExecutableGuiEnabled(false);
  setPatternGuiEnabled(false);
  markClean();
}

bool OpenWithManagerDialog::apply()
{
  // Check that all factories are using valid executables:
  int index = -1;
  foreach (const ProgrammableOpenWithActionFactory &factory, m_factories) {
    ++index;
    QString reason;
    QString name = factory.name();
    QString executable = factory.executable();
    QString executableFilePath;
    switch (validateExecutable(executable, executableFilePath)) {
    case ExecOk:
      break;
    case ExecNotExec:
      reason = tr("File is not executable: %1").arg(executableFilePath);
      break;
    case ExecInvalidPath:
      reason = tr("File not found in specified path.");
      break;
    case ExecNotFound:
      reason = tr("No file in system path named '%1'.").arg(executable);
      break;
    }

    if (reason.isEmpty())
      continue;

    QMessageBox::StandardButton response =
        QMessageBox::warning(this, name,
                             tr("An issue was found with the executable for "
                                "'%1':\n\n%2\n\nWould you like to change the "
                                "executable now?").arg(name, reason),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes);

    if (response == QMessageBox::No)
      continue;

    ui->tableExec->selectRow(index);
    ui->editExec->selectAll();
    ui->editExec->setFocus();
    return false;
  }

  // Delete the original factories from the manager and replace them with our
  // new ones
  ActionFactoryManager *manager = ActionFactoryManager::getInstance();
  foreach (JobActionFactory *factory, m_origFactories)
    manager->removeFactory(factory);
  foreach (ProgrammableOpenWithActionFactory factory, m_factories)
    manager->addFactory(new ProgrammableOpenWithActionFactory(factory));

  loadFactories();

  return true;
}

void OpenWithManagerDialog::accept()
{
  if (!apply())
    return;

  reset();
  QDialog::accept();
}

void OpenWithManagerDialog::reject()
{
  reset();
  QDialog::reject();
}

void OpenWithManagerDialog::closeEvent(QCloseEvent *e)
{
  // Ensure that all forms are submitted
  if (QWidget *focus = this->focusWidget())
    focus->clearFocus();

  if (m_dirty) {
    // apply or discard changes?
    QMessageBox::StandardButton reply =
        QMessageBox::warning(this, tr("Unsaved changes"),
                             tr("Your changes have not been saved. Would you "
                                "like to save or discard them?"),
                             QMessageBox::Save | QMessageBox::Discard |
                             QMessageBox::Cancel,
                             QMessageBox::Save);

    switch (reply) {
    case QMessageBox::Cancel:
      e->ignore();
      return;
    case QMessageBox::Save:
      if (!apply())
        return;
    case QMessageBox::NoButton:
    case QMessageBox::Discard:
    default:
      break;
    }
  }

  QDialog::closeEvent(e);
}

void OpenWithManagerDialog::buttonBoxClicked(QAbstractButton *button)
{
  // "Ok" and "Cancel" are directly connected to accept() and reject(), so only
  // check for "apply" here:
  if (button == ui->buttonBox->button(QDialogButtonBox::Apply))
    apply();
}

void OpenWithManagerDialog::markClean()
{
  m_dirty = false;
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}

void OpenWithManagerDialog::markDirty()
{
  m_dirty = true;
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void OpenWithManagerDialog::addExecutable()
{
  QModelIndexList sel = selectedExecutableIndices();
  int index = -1;
  if (sel.size() == m_execModel->columnCount())
    index = sel.first().row();

  if (index + 1 > m_execModel->rowCount(QModelIndex()) || index < 0)
    index = m_execModel->rowCount(QModelIndex());

  m_execModel->insertRow(index);

  ui->tableExec->selectionModel()->select(
        m_execModel->index(index, 0),
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void OpenWithManagerDialog::removeExecutable()
{
  QModelIndexList sel = selectedExecutableIndices();
  if (sel.size() != m_execModel->columnCount())
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
      QString absoluteFilePath = searchSystemPathForFile(info.fileName());
      // Found the path; initialize the file dialog to it
      if (!absoluteFilePath.isEmpty()) {
        ui->editExec->setText(absoluteFilePath);
        m_execMapper->submit();
        initialPath = absoluteFilePath;
      }
    }
  }

  // If we didn't find a path above, just use the user's home directory.
  if (initialPath.isEmpty())
    initialPath = QDir::homePath();

  QString newFilePath = QFileDialog::getOpenFileName(
        this, tr("Select executable"), initialPath);

  if (!newFilePath.isEmpty()) {
    ui->editExec->setText(newFilePath);
    m_execMapper->submit();
  }

  testExecutable();
}

OpenWithManagerDialog::ExecutableStatus
OpenWithManagerDialog::validateExecutable(const QString &executable)
{
  QString tmp;
  return validateExecutable(executable, tmp);
}

OpenWithManagerDialog::ExecutableStatus
OpenWithManagerDialog::validateExecutable(const QString &executable,
                                          QString &executableFilePath)
{
  QFileInfo info(executable);
  if (info.isAbsolute()) {
    executableFilePath = info.absoluteFilePath();
    if (!info.exists() || !info.isFile())
      return ExecInvalidPath;
    else if (!info.isExecutable())
      return ExecNotExec;
    return ExecOk;
  }
  else {
    executableFilePath = searchSystemPathForFile(executable);
    info = QFileInfo(executableFilePath);
    if (executableFilePath.isEmpty() || !info.isFile())
      return ExecNotFound;
    else if (!info.isExecutable())
      return ExecNotExec;
    return ExecOk;
  }
}

void OpenWithManagerDialog::testExecutable()
{
  switch (validateExecutable(ui->editExec->text())) {
  case ExecOk:
    testExecutableMatch();
    break;
  default:
  case ExecNotExec:
  case ExecInvalidPath:
  case ExecNotFound:
    testExecutableNoMatch();
    break;
  }
}

void OpenWithManagerDialog::testExecutableMatch()
{
  QPalette pal;
  pal.setColor(QPalette::Text, Qt::black);
  ui->editExec->setPalette(pal);
}

void OpenWithManagerDialog::testExecutableNoMatch()
{
  QPalette pal;
  pal.setColor(QPalette::Text, Qt::red);
  ui->editExec->setPalette(pal);
}

void OpenWithManagerDialog::executableSelectionChanged()
{
  // Get selected executable
  QModelIndexList sel = selectedExecutableIndices();
  int index = -1;
  if (sel.size() == m_execModel->columnCount())
    index = sel.first().row();

  // If valid, set the regexp list
  if (index >= 0 && index < m_execModel->rowCount(QModelIndex())) {
    setExecutableGuiEnabled(true);
    setPatternGuiEnabled(true);
    m_patternModel->setRegExps(&m_factories[index].recognizedFilePatternsRef());
    m_patternMapper->toFirst();
  }
  // otherwise, clear the regexp list and disable the pattern GUI
  else {
    setExecutableGuiEnabled(false);
    setPatternGuiEnabled(false);
    m_patternModel->setRegExps(NULL);
  }

  // Update the execMapper
  if (sel.size())
    m_execMapper->setCurrentIndex(sel.first().row());
}

void OpenWithManagerDialog::setExecutableGuiEnabled(bool enable)
{
  ui->editExec->setEnabled(enable);
  ui->labelExec->setEnabled(enable);
  ui->pushExec->setEnabled(enable);
  ui->editExec->setEnabled(enable);
  ui->editName->setEnabled(enable);
  ui->labelName->setEnabled(enable);
  if (!enable) {
    ui->editExec->blockSignals(true);
    ui->editExec->clear();
    ui->editExec->blockSignals(false);
    ui->editName->blockSignals(true);
    ui->editName->clear();
    ui->editName->blockSignals(false);
  }
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
  ui->tablePattern->horizontalHeader()->setResizeMode(
        OpenWithPatternModel::PatternCol, QHeaderView::Stretch);
}

void OpenWithManagerDialog::setPatternGuiEnabled(bool enable)
{
  ui->groupPattern->setEnabled(enable);
  ui->labelPattern->setEnabled(enable);
  ui->editPattern->setEnabled(enable);
  ui->comboMatch->setEnabled(enable);
  ui->checkCaseSensitive->setEnabled(enable);
  ui->pushApplyPattern->setEnabled(enable);
  ui->pushRevertPattern->setEnabled(enable);
  // also clear edits if disabling:
  if (!enable) {
    ui->editPattern->blockSignals(true);
    ui->editPattern->clear();
    ui->editPattern->blockSignals(false);
    ui->comboMatch->blockSignals(true);
    ui->comboMatch->setCurrentIndex(0);
    ui->comboMatch->blockSignals(false);
    ui->checkCaseSensitive->blockSignals(false);
    ui->checkCaseSensitive->setEnabled(enable);
    ui->checkCaseSensitive->blockSignals(true);
  }
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
  pal.setColor(QPalette::Text, Qt::black);
  ui->editTest->setPalette(pal);
}

void OpenWithManagerDialog::testTextNoMatch()
{
  QPalette pal;
  pal.setColor(QPalette::Text, Qt::red);
  ui->editTest->setPalette(pal);
}

QString OpenWithManagerDialog::searchSystemPathForFile(const QString &exec)
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
    QFileInfo info(QUrl::fromLocalFile(path + "/" + exec).toLocalFile());
    if (!info.exists()
        || !info.isFile()) {
      continue;
    }
    result = info.absoluteFilePath();
    break;
  }

  return result;
}

QModelIndexList OpenWithManagerDialog::selectedExecutableIndices() const
{
  return ui->tableExec->selectionModel()->selectedIndexes();
}

QModelIndexList OpenWithManagerDialog::selectedPatternIndices() const
{
  return ui->tablePattern->selectionModel()->selectedIndexes();
}

ProgrammableOpenWithActionFactory *OpenWithManagerDialog::selectedFactory()
{
  QModelIndexList sel = selectedExecutableIndices();
  if (sel.size() != m_execModel->columnCount())
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
  switch (ev->key()) {
  // By default, the escape key bypasses the close event, but we still want to
  // check if the settings widget is dirty.
  case Qt::Key_Escape:
    ev->accept();
    close();
    break;

  // Disable forwarding of enter and return to Ok/Cancel buttons. Too easy to
  // accidentally close the dialog while modifying line edits.
  case Qt::Key_Return:
  case Qt::Key_Enter:
    break;

  default:
    QDialog::keyPressEvent(ev);
  }
}

} // end namespace MoleQueue
