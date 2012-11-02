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

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QSettings>

#include <json/json.h>

namespace MoleQueue
{

Logger *Logger::m_instance = NULL;

Logger::Logger() :
  QObject(),
  m_printDebugMessages(false),
  m_printNotifications(false),
  m_printWarnings(false),
  m_printErrors(false),
  m_maxEntries(1000),
  m_newErrorCount(0),
  m_silenceNewErrors(false),
  m_logFile(NULL)
{
  // Call destructor when program exits
  atexit(&cleanUp);

  QFile *lfile = logFile();

  if (lfile) {

    if (!lfile->open(QFile::ReadOnly | QFile::Text)) {
      QSettings settings;
      if (settings.value("logWritten", false).toBool()) {
        qWarning() << "MoleQueue::Logger::~Logger() -- Cannot open log "
                      "file " + lfile->fileName() + "; cannot read log.";
      }
      return;
    }

    QByteArray logData = lfile->readAll();
    lfile->close();

    Json::Value root;
    Json::Reader reader;
    reader.parse(logData.constData(), logData.constData() + logData.size(),
                 root);

    if (root["maxEntries"].isIntegral())
      m_maxEntries = root["maxEntries"].asInt();

    const Json::Value &entries = root["entries"];
    if (entries.isArray()) {
      for (Json::ValueIterator it = entries.begin(), it_end = entries.end();
           it != it_end; ++it) {
        m_log.push_back(LogEntry(*it));
      }
    }
  } // end if (lfile)
}

Logger::~Logger()
{
  QFile *lfile = logFile();

  if (lfile) {

    if (!lfile->open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
      qWarning() << "MoleQueue::Logger::~Logger() -- Cannot create log "
                    "file" + lfile->fileName() + "; cannot save log.";
      return;
    }

    Json::Value root(Json::objectValue);
    root["maxEntries"] = m_maxEntries;

    Json::Value entriesArray(Json::arrayValue);
    foreach(const LogEntry &entry, m_log) {
      Json::Value entryObject(Json::objectValue);
      entry.writeSettings(entryObject);
      entriesArray.append(entryObject);
    }
    root["entries"] = entriesArray;

    lfile->write(root.toStyledString().c_str());
    lfile->close();

    QSettings settings;
    settings.setValue("logWritten", true);
  } // end if (lfile)
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

QFile *Logger::logFile()
{
  if (!m_logFile) {
    QSettings settings;
    QString workDir = settings.value("workingDirectoryBase").toString();

    if (workDir.isEmpty()) {
      qWarning() << "MoleQueue::Logger::~Logger() -- Cannot determine working "
                    "directory.";
      return NULL;
    }

    QDir logDir(workDir + "/log");
    if (!logDir.exists()) {
      if (!logDir.mkpath(logDir.absolutePath())) {
        qWarning() << "MoleQueue::Logger::~Logger() -- Cannot create log "
                      "directory" + logDir.absolutePath();
        return NULL;
      }
    }

    m_logFile = new QFile(logDir.absoluteFilePath("log.json"));
  }

  if (m_logFile) {
    if (m_logFile->isOpen())
      m_logFile->close();
  }

  return m_logFile;
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
