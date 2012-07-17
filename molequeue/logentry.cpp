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

#include "logentry.h"

#include <QtCore/QSettings>

namespace MoleQueue
{

LogEntry::LogEntry(LogEntryType type, const QString &message_,
                   const IdType &moleQueueId_)
  : m_message(message_),
    m_moleQueueId(moleQueueId_),
    m_entryType(type)
{
}

LogEntry::LogEntry(QSettings &settings)
  : m_message(settings.value("message").toString()),
    m_moleQueueId(
      static_cast<IdType>(settings.value("moleQueueId").toULongLong())),
    m_entryType(static_cast<LogEntryType>(settings.value("entryType").toInt())),
    m_timeStamp(QDateTime::fromString(settings.value("timeStamp").toString()))
{
}

LogEntry::LogEntry(const LogEntry &other)
  : m_message(other.m_message),
    m_moleQueueId(other.m_moleQueueId),
    m_entryType(other.m_entryType),
    m_timeStamp(other.m_timeStamp)
{
}

LogEntry::~LogEntry()
{
}

void LogEntry::writeSettings(QSettings &settings) const
{
  settings.setValue("message", m_message);
  settings.setValue("moleQueueId", static_cast<quint64>(m_moleQueueId));
  settings.setValue("entryType", static_cast<int>(m_entryType));
  settings.setValue("timeStamp", m_timeStamp.toString());
}

void LogEntry::setTimeStamp()
{
  m_timeStamp = QDateTime::currentDateTime();
}

const QDateTime &LogEntry::timeStamp() const
{
  return m_timeStamp;
}

} // namespace MoleQueue
