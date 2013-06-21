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

#ifndef MOLEQUEUE_LOGENTRY_H
#define MOLEQUEUE_LOGENTRY_H

#include <molequeue/app/molequeueglobal.h>

#include <QtCore/QDateTime>
#include <QtCore/QString>

class QJsonObject;

namespace MoleQueue
{
class Logger;

/**
 * @class LogEntry logentry.h <molequeue/logentry.h>
 * @brief Message and metadata for log messages.
 * @author David C. Lonie
 *
 * Each LogEntry object represents an entry in the MoleQueue log. LogEntries
 * fall into one of four categories:
 * - DebugMessage: Verbose debugging information.
 * - Notification: Routine information that is relevant to the user.
 * - Warning: Non-routine information that is relevant to the user, but does not
 *   indicate a serious problem.
 * - Error: Serious problem that will affect either the MoleQueue application
 *   or a Job's ability to perform properly.
 *
 * The easiest way to add new entries to the log is to use the static functions
 * in Logger:
 * - Logger::logDebugMessage(QString message, IdType moleQueueId)
 * - Logger::logNotification(QString message, IdType moleQueueId)
 * - Logger::logWarning(QString message, IdType moleQueueId)
 * - Logger::logError(QString message, IdType moleQueueId)
 *
 * Each LogEntry contains a user-friendly message, an LogEntryType to identify
 * the type of log entry, an optional MoleQueue id for any associate Job, and
 * a timestamp, which is set by the Logger when the entry is added.
 *
 * @see Logger
 */
class LogEntry
{
public:
  /// Enumeration of different types of log entries.
  enum LogEntryType {
    /// Verbose debugging information.
    DebugMessage = 0,
    /// Routine information that is relevant to the user.
    Notification,
    /// Non-routine information that is relevant to the user, but does not
    /// indicate a serious problem.
    Warning,
    /// Serious problem that will affect either the MoleQueue application
    /// or a Job's ability to perform properly.
    Error
  };

  /**
   * @brief LogEntry Construct a new log entry.
   * @param type Type of log message.
   * @param message_ Descriptive user-visible message for log.
   * @param moleQueueId_ MoleQueue id of any associated job.
   * @see Logger::addDebugMessage
   * @see Logger::addNotification
   * @see Logger::addWarning
   * @see Logger::addError
   */
  LogEntry(LogEntryType type, const QString &message_,
           const IdType &moleQueueId_ = InvalidId);
  /// Copy the LogEntry @a other into a new LogEntry
  LogEntry(const LogEntry &other);
  /// Destroy the log entry
  virtual ~LogEntry();

  /// @return The type of log message.
  LogEntryType entryType() const {return m_entryType;}

  /// @return True if this message has type @a type.
  bool isEntryType(LogEntryType type) const {return m_entryType == type;}

  /// A user-friendly log message.
  void setMessage(const QString &message_) { m_message = message_; }

  /// A user-friendly log message
  QString message() const { return m_message; }

  /// The MoleQueue id of the associated job (if any, InvalidId otherwise).
  void setMoleQueueId(IdType moleQueueId_) { m_moleQueueId = moleQueueId_; }

  /// The MoleQueue id of the associated job (if any, InvalidId otherwise).
  IdType moleQueueId() const { return m_moleQueueId; }

  // Get the timestamp on the LogEntry.
  const QDateTime & timeStamp() const;

  friend class MoleQueue::Logger;

protected:

  /// Initialize from data in the QJsonObject.
  LogEntry(const QJsonObject &json);

  /// Write this entry's settings to the QJsonObject.
  void writeSettings(QJsonObject &root) const;

  /// Set the timestamp on this LogEntry to the current time.
  void setTimeStamp();

private:
  QString m_message;
  IdType m_moleQueueId;
  LogEntryType m_entryType;
  QDateTime m_timeStamp;

};

} // namespace MoleQueue

#endif // MOLEQUEUE_LOGENTRY_H
