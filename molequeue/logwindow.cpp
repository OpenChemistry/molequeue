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

#include "logwindow.h"
#include "ui_logwindow.h"

#include "logger.h"

#include <QtCore/QSettings>

#include <QtGui/QBrush>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QTextBlockFormat>
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtGui/QTextEdit>
#include <QtGui/QTextFrame>
#include <QtGui/QVBoxLayout>

namespace MoleQueue {

LogWindow::LogWindow(QWidget *theParent) :
  QMainWindow(theParent),
  ui(new Ui::LogWindow),
  m_log(NULL),
  m_maxEntries(NULL),
  m_logEntryBlockFormat(new QTextBlockFormat()),
  m_timeStampFormat(new QTextCharFormat()),
  m_debugMessageFormat(new QTextCharFormat()),
  m_notificationFormat(new QTextCharFormat()),
  m_warningFormat(new QTextCharFormat()),
  m_errorFormat(new QTextCharFormat()),
  m_moleQueueIdFormat(new QTextCharFormat())
{
  createUi();

  // Restore geometry
  QSettings settings;
  settings.beginGroup("logWindow");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
  settings.endGroup();

  setupFormats();

  connect(Logger::getInstance(), SIGNAL(newLogEntry(MoleQueue::LogEntry)),
          this, SLOT(addLogEntry(MoleQueue::LogEntry)));

  initializeLogText();
}

LogWindow::~LogWindow()
{
  // Store geometry
  QSettings settings;
  settings.beginGroup("logWindow");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  settings.endGroup();

  delete ui;
  delete m_logEntryBlockFormat;
  delete m_timeStampFormat;
  delete m_debugMessageFormat;
  delete m_notificationFormat;
  delete m_warningFormat;
  delete m_errorFormat;
  delete m_moleQueueIdFormat;
}

void LogWindow::changeEvent(QEvent *e)
{
  if (e->type() == QEvent::ActivationChange && isActiveWindow())
    Logger::resetNewErrorCount();

  QMainWindow::changeEvent(e);
}

void LogWindow::closeEvent(QCloseEvent *e)
{
  Logger::silenceNewErrors(false);
  Logger::resetNewErrorCount();

  QMainWindow::closeEvent(e);
}

void LogWindow::hideEvent(QHideEvent *e)
{
  Logger::silenceNewErrors(false);
  Logger::resetNewErrorCount();

  QMainWindow::hideEvent(e);
}

void LogWindow::showEvent(QShowEvent *e)
{
  Logger::silenceNewErrors(true);
  Logger::resetNewErrorCount();

  QMainWindow::showEvent(e);
}

void LogWindow::addLogEntry(const LogEntry &entry)
{
  QString entryType;
  QTextCharFormat *entryFormat;
  switch (entry.entryType()) {
  case LogEntry::DebugMessage:
    entryType = tr("Debug");
    entryFormat = m_debugMessageFormat;
    break;
  case LogEntry::Notification:
    entryType = tr("Notification");
    entryFormat = m_notificationFormat;
    break;
  case LogEntry::Warning:
    entryType = tr("Warning");
    entryFormat = m_warningFormat;
    break;
  case LogEntry::Error:
    entryType = tr("Error");
    entryFormat = m_errorFormat;
    break;
  default:
    entryType = tr("LogEntry");
    entryFormat = m_debugMessageFormat;
  }

  QTextDocument *doc = m_log->document();
  QTextCursor cur(doc);
  cur.beginEditBlock();
  cur.movePosition(QTextCursor::Start);
  cur.insertBlock(*m_logEntryBlockFormat);
  cur.insertText(entry.timeStamp().toString(), *m_timeStampFormat);
  cur.insertText(" ");
  cur.insertText(QString("%1").arg(entryType, -12), *entryFormat);
  cur.insertText(" ");
  if (entry.moleQueueId() == InvalidId) {
    cur.insertText(tr("Job %1").arg("N/A", -5), *m_moleQueueIdFormat);
  }
  else {
    cur.insertText(tr("Job %1").arg(entry.moleQueueId(), -5),
                   *m_moleQueueIdFormat);
  }
  cur.insertText(" ");
  cur.insertText(entry.message(), *entryFormat);
  cur.endEditBlock();
}

void LogWindow::clearLog()
{
  Logger::clear();
  initializeLogText();
}

void LogWindow::changeMaxEntries()
{
  Logger::getInstance()->setMaxEntries(m_maxEntries->value());
}

void LogWindow::createUi()
{
  ui->setupUi(this);

  QWidget *widget = new QWidget (this);
  setCentralWidget(widget);
  QVBoxLayout *mainLayout = new QVBoxLayout (widget);

  m_log = new QTextEdit(this);
  m_log->setReadOnly(true);
  mainLayout->addWidget(m_log);

  QHBoxLayout *logSettingsLayout = new QHBoxLayout ();

  QPushButton *clearLogButton = new QPushButton(tr("&Clear log"), this);
  connect(clearLogButton, SIGNAL(clicked()), this, SLOT(clearLog()));
  logSettingsLayout->addWidget(clearLogButton);

  logSettingsLayout->addStretch();

  QLabel *maxEntriesLabel = new QLabel (tr("&Maximum log size:"), this);
  m_maxEntries = new QSpinBox (this);
  m_maxEntries->setRange(0, 10000);
  m_maxEntries->setValue(Logger::maxEntries());
  m_maxEntries->setSuffix(QString(" ") + tr("entries"));
  connect(m_maxEntries, SIGNAL(editingFinished()),
          this, SLOT(changeMaxEntries()));
  maxEntriesLabel->setBuddy(m_maxEntries);
  logSettingsLayout->addWidget(maxEntriesLabel);
  logSettingsLayout->addWidget(m_maxEntries);

  mainLayout->addLayout(logSettingsLayout);
}

void LogWindow::setupFormats()
{
  // Use a hanging indent:
  m_logEntryBlockFormat->setTextIndent(-40);
  m_logEntryBlockFormat->setIndent(1);
  m_logEntryBlockFormat->setBottomMargin(5);

  m_timeStampFormat->setForeground(QBrush(Qt::blue));
  m_timeStampFormat->setFontFamily("monospace");

  m_debugMessageFormat->setForeground(QBrush(Qt::darkGray));
  m_debugMessageFormat->setFontFamily("monospace");

  m_notificationFormat->setForeground(QBrush(Qt::darkYellow));
  m_notificationFormat->setFontWeight(QFont::Bold);
  m_notificationFormat->setFontFamily("monospace");

  m_warningFormat->setForeground(QBrush(Qt::darkRed));
  m_warningFormat->setFontWeight(QFont::Bold);
  m_warningFormat->setFontFamily("monospace");

  m_errorFormat->setForeground(QBrush(Qt::red));
  m_errorFormat->setFontWeight(QFont::Bold);
  m_errorFormat->setFontFamily("monospace");

  m_moleQueueIdFormat->setForeground(QBrush(Qt::darkCyan));
  m_moleQueueIdFormat->setFontFamily("monospace");
}

void LogWindow::initializeLogText()
{
  m_log->clear();
  foreach (const LogEntry &entry, Logger::log())
    addLogEntry(entry);

  m_log->moveCursor(QTextCursor::Start);
  m_log->ensureCursorVisible();
}

} // namespace MoleQueue
