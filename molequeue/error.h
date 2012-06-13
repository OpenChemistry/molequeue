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

#ifndef ERROR_H
#define ERROR_H

#include "molequeueglobal.h"
#include "object.h" // For QPointer

#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QVariant>

namespace MoleQueue
{

class Error
{
public:

  /// Enum defining the type of error that occurred.
  enum ErrorType {
    /// Catch-all error type.
    MiscError = 0,
    /// Error communicating with remote server (e.g. ssh).
    NetworkError,
    /// Error communicating between processes (e.g. socket, zeroMQ, etc).
    IPCError,
    /// Error interacting with filesystem (e.g. invalid permissions, etc).
    FileSystemError,
    /// Error involving a local or remote queue.
    QueueError,
    /// Error involving program execution.
    ProgramError
  };

  /**
   * Construct an error with the indicated properties.
   *
   * @param message_ Descriptive message describing the error.
   * @param type_ Type of error that occurred. Default: MiscError
   * @param sender_ Object from which the error originated. Default: NULL
   * @param moleQueueId_ MoleQueue id of Job associated with error. 0 indicates
   * no associated Job. Default: 0
   * @param data_ Optional supplemental data. Default: QVariant()
   */
  Error(const QString &message_ = "", ErrorType type_ = MiscError,
        Object *sender_ = NULL, IdType moleQueueId_ = 0,
        const QVariant data_ = QVariant());

  /**
   * Copy the members of @a other into a new error.
   * @param other Other Error object.
   */
  Error(const Error &other);

  void setMessage(const QString &message_) { m_message = message_; }
  QString message() const { return m_message; }

  void setType(ErrorType type_) { m_type = type_; }
  ErrorType type() const { return m_type; }

  void setSender(Object *sender_) { m_sender = sender_; }
  Object * sender() const { return m_sender; }

  void setMoleQueueId(IdType moleQueueId_) { m_moleQueueId = moleQueueId_; }
  IdType moleQueueId() const { return m_moleQueueId; }

  void setData(const QVariant data_) { m_data = data_; }
  QVariant data() const { return m_data; }

protected:
  QString m_message;
  ErrorType m_type;
  QPointer<Object> m_sender;
  IdType m_moleQueueId;
  QVariant m_data;
};

} // end namespace MoleQueue

#endif // ERROR_H
