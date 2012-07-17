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

#ifndef MOLEQUEUE_LOGWINDOW_H
#define MOLEQUEUE_LOGWINDOW_H

#include <QtGui/QMainWindow>

class QSpinBox;
class QTextBlockFormat;
class QTextCharFormat;
class QTextEdit;

namespace Ui {
class LogWindow;
}

namespace MoleQueue
{
class LogEntry;

/// Window that displays log contents
class LogWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit LogWindow(QWidget *theParent = 0);
  ~LogWindow();

private slots:
  void addLogEntry(const MoleQueue::LogEntry &);
  void clearLog();
  void changeMaxEntries();

private:
  void createUi();
  void setupFormats();
  void initializeLogText();

  Ui::LogWindow *ui;

  QTextEdit *m_log;
  QSpinBox *m_maxEntries;

  QTextBlockFormat *m_logEntryBlockFormat;
  QTextCharFormat *m_timeStampFormat;
  QTextCharFormat *m_debugMessageFormat;
  QTextCharFormat *m_notificationFormat;
  QTextCharFormat *m_warningFormat;
  QTextCharFormat *m_errorFormat;
  QTextCharFormat *m_moleQueueIdFormat;
};


} // namespace MoleQueue

#endif // MOLEQUEUE_LOGWINDOW_H
