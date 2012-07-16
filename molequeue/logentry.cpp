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

#include <QtCore/QDateTime>

namespace MoleQueue
{

LogEntry::LogEntry(LogEntryType type, const QString &message_,
                   const IdType &moleQueueId_)
  : m_message(message_),
    m_moleQueueId(moleQueueId_),
    m_entryType(type),
    m_timeStamp(new QDateTime())
{
}

LogEntry::LogEntry(const LogEntry &other)
  : m_message(other.m_message),
    m_moleQueueId(other.m_moleQueueId),
    m_entryType(other.m_entryType),
    m_timeStamp(new QDateTime (*other.m_timeStamp))
{
}

LogEntry::~LogEntry()
{
  delete m_timeStamp;
}

void LogEntry::setTimeStamp()
{
  *m_timeStamp = QDateTime::currentDateTime();
}

const QDateTime &LogEntry::timeStamp() const
{
  return *m_timeStamp;
}

} // namespace MoleQueue
