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

#ifndef MOLEQUEUE_LOGGER_H
#define MOLEQUEUE_LOGGER_H

#include <QtCore/QObject>

#include "logentry.h"

namespace MoleQueue
{

/**
 * @class Logger logger.h <molequeue/logger.h>
 * @brief Manage log messages.
 * @author David C. Lonie
 *
 * The singleton Logger class is used to handle log messages in MoleQueue. Log
 * messages are represented as objects consisting of a user-friendly string,
 * an enum value representing a subtype, and an optional MoleQueue id for any
 * associated job.
 *
 * There are four levels of log messages:
 * - DebugMessage: Verbose debugging information.
 * - Notification: Routine information that is relevant to the user.
 * - Warning: Non-routine information that is relevant to the user, but does not
 *   indicate a serious problem.
 * - Error: Serious problem that will affect either the MoleQueue application
 *   or a Job's ability to perform properly.
 *
 * New log entries can be submitted using the static Logger::addLogEntry method.
 * Each new log entry causes the newLogEntry signal to be emitted, as well as
 * one of newDebugMessage, newNotification, newWarning, or newError, depending
 * on the LogEntry type. Details of new log entries will be automatically
 * sent to qDebug() (DebugMessage, Notification) or qWarning() (Warning, Error)
 * depending on the values of the print* methods. By default, all log entries
 * except for DebugMessages are printed.
 */
class Logger : public QObject
{
  Q_OBJECT
public:
  ~Logger();
  /// @return The singleton Logger instance
  static Logger *getInstance();

  /// @return Whether or not to print debugging messages to qDebug.
  /// Default: false
  bool printDebugMessages() const { return m_printDebugMessages; }

  /// @return Whether or not to print notifications to qDebug. Default: true
  bool printNotifications() const { return m_printNotifications; }

  /// @return Whether or not to print warnings to qWarning. Default: true
  bool printWarnings() const { return m_printWarnings; }

  /// @return Whether or not to print errors to qWarning. Default: true
  bool printErrors() const { return m_printErrors; }

signals:

  /// Emitted when a new debugging message has been added to the log.
  void newDebugMessage(const MoleQueue::LogEntry &debug);

  /// Emitted when a new notification has been added to the log.
  void newNotification(const MoleQueue::LogEntry &notif);

  /// Emitted when a new warning has been added to the log.
  void newWarning(const MoleQueue::LogEntry &warning);

  /// Emitted when a new error has been added to the log.
  void newError(const MoleQueue::LogEntry &error);

  /// Emitted when any new log entry is added to the log.
  void newLogEntry(const MoleQueue::LogEntry &entry);

public slots:

  /// Add @a entry to the log.
  static void addLogEntry(MoleQueue::LogEntry &entry)
  {
    Logger::getInstance()->handleNewLogEntry(entry);
  }

  /// Add a new log entry to the log.
  static void addLogEntry(LogEntry::LogEntryType type, const QString &message,
                          const IdType &moleQueueId = InvalidId)
  {
    LogEntry entry(type, message, moleQueueId);
    Logger::addLogEntry(entry);
  }

  /// Add a new debugging message to the log.
  static void addDebugMessage(const QString &message,
                              const IdType &moleQueueId = InvalidId)
  {
    LogEntry entry(LogEntry::DebugMessage, message, moleQueueId);
    Logger::addLogEntry(entry);
  }

  /// Add a new notification to the log.
  static void addNotification(const QString &message,
                              const IdType &moleQueueId = InvalidId)
  {
    LogEntry entry(LogEntry::Notification, message, moleQueueId);
    Logger::addLogEntry(entry);
  }

  /// Add a new warning to the log.
  static void addWarning(const QString &message,
                         const IdType &moleQueueId = InvalidId)
  {
    LogEntry entry(LogEntry::Warning, message, moleQueueId);
    Logger::addLogEntry(entry);
  }

  /// Add a new error to the log.
  static void addError(const QString &message,
                       const IdType &moleQueueId = InvalidId)
  {
    LogEntry entry(LogEntry::Error, message, moleQueueId);
    Logger::addLogEntry(entry);
  }

  /// @param print Whether or not to print debugging messages to qDebug.
  /// Default: false
  void setPrintDebugMessages(bool print) { m_printDebugMessages = print; }

  /// @param print Whether or not to print notifications to qDebug. Default:
  /// true
  void setPrintNotifications(bool print) { m_printNotifications = print; }

  /// @param print Whether or not to print warnings to qWarning. Default: true
  void setPrintWarnings(bool print) { m_printWarnings = print; }

  /// @param print Whether or not to print errors to qWarning. Default: true
  void setPrintErrors(bool print) { m_printErrors = print; }

  /// @return A list of all log entries.
  QList<LogEntry> getLog() const { return m_log;}

private:
  Logger();
  static Logger *m_instance;

  void handleNewLogEntry(LogEntry &entry);
  void handleNewDebugMessage(const MoleQueue::LogEntry &debug);
  void handleNewNotification(const MoleQueue::LogEntry &notif);
  void handleNewWarning(const MoleQueue::LogEntry &warning);
  void handleNewError(const MoleQueue::LogEntry &error);

  bool m_printDebugMessages;
  bool m_printNotifications;
  bool m_printWarnings;
  bool m_printErrors;

  QList<LogEntry> m_log;
};

} // namespace MoleQueue

#endif // MOLEQUEUE_LOGGER_H
