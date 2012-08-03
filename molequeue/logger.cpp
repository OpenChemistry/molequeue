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

#include "logger.h"

#include "logentry.h"

#include <QtCore/QSettings>

namespace MoleQueue
{

Logger *Logger::m_instance = NULL;

Logger::Logger() :
  QObject(),
  m_printDebugMessages(false),
  m_printNotifications(false),
  m_printWarnings(false),
  m_printErrors(false),
  m_newErrorCount(0),
  m_silenceNewErrors(false)
{
  // Call destructor when program exits
  atexit(&cleanUp);

  QSettings settings;
  settings.beginGroup("logger");
  m_maxEntries = settings.value("maxEntries", 1000).toInt();
  int numEntries = settings.beginReadArray("entries");
  for (int i = 0; i < numEntries; ++i) {
    settings.setArrayIndex(i);
    m_log.push_back(LogEntry(settings));
  }
  settings.endArray();
  settings.endGroup();
}

Logger::~Logger()
{
  QSettings settings;
  settings.beginGroup("logger");
  settings.setValue("maxEntries", m_maxEntries);
  settings.beginWriteArray("entries", m_log.size());
  int i = 0;
  foreach(const LogEntry &entry, m_log) {
    settings.setArrayIndex(i++);
    entry.writeSettings(settings);
  }
  settings.endArray();
  settings.endGroup();
}

void Logger::resetNewErrorCount()
{
  Logger *instance = Logger::getInstance();
  if (instance->m_newErrorCount == 0)
    return;

  emit instance->newErrorCountReset();
  instance->m_newErrorCount = 0;
}

void Logger::cleanUp()
{
  delete m_instance;
  m_instance = NULL;
}

Logger *Logger::getInstance()
{
  if (!m_instance)
    m_instance = new Logger ();

  return m_instance;
}

void Logger::handleNewLogEntry(LogEntry &entry)
{
  entry.setTimeStamp();
  m_log.push_back(entry);

  trimLog();

  switch (entry.entryType()) {
  case LogEntry::DebugMessage:
    handleNewDebugMessage(entry);
    break;
  case LogEntry::Notification:
    handleNewNotification(entry);
    break;
  case LogEntry::Warning:
    handleNewWarning(entry);
    break;
  case LogEntry::Error:
    handleNewError(entry);
    break;
  }
  emit newLogEntry(entry);
}

inline void Logger::handleNewDebugMessage(const MoleQueue::LogEntry &debug)
{
  if (m_printDebugMessages) {
    qDebug() << "Debugging message:"
             << "Message: " << debug.message()
             << "MoleQueueId: (" << debug.moleQueueId() << ")";
  }
  emit newDebugMessage(debug);
}

inline void Logger::handleNewNotification(const MoleQueue::LogEntry &notif)
{
  if (m_printNotifications) {
    qDebug() << "Notification:"
             << "Message: " << notif.message()
             << "MoleQueueId: (" << notif.moleQueueId() << ")";
  }
  emit newNotification(notif);
}

inline void Logger::handleNewWarning(const MoleQueue::LogEntry &warning)
{
  if (m_printWarnings) {
    qDebug() << "Warning:"
             << "Message: " << warning.message()
             << "MoleQueueId: (" << warning.moleQueueId() << ")";
  }
  emit newWarning(warning);
}

inline void Logger::handleNewError(const MoleQueue::LogEntry &error)
{
  if (m_printErrors) {
    qDebug() << "Error occurred:"
             << "Message: " << error.message()
             << "MoleQueueId: (" << error.moleQueueId() << ")";
  }

  ++m_newErrorCount;

  emit newError(error);

  if (!m_silenceNewErrors && m_newErrorCount == 1)
    emit firstNewErrorOccurred();
}

void Logger::trimLog()
{
  if (m_log.size() > m_maxEntries) {
    m_log.erase(m_log.begin(),
                m_log.begin() + (m_log.size() - m_maxEntries));
  }
}

} // namespace MoleQueue
