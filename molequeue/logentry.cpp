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

#include "molequeueglobal.h"

#include <json/json.h>

namespace MoleQueue
{

LogEntry::LogEntry(LogEntryType type, const QString &message_,
                   const IdType &moleQueueId_)
  : m_message(message_),
    m_moleQueueId(moleQueueId_),
    m_entryType(type)
{
}

LogEntry::LogEntry(const Json::Value &json)
  : m_message(json["message"].isString() ? json["message"].asCString()
                                         : "Invalid JSON!"),
    m_moleQueueId(toIdType(json["moleQueueId"])),
    m_entryType(json["entryType"].isIntegral()
                ? static_cast<LogEntryType>(json["entryType"].asInt())
                : Error),
    m_timeStamp(json["time"].isString()
                ? QDateTime::fromString(json["time"].asCString())
                : QDateTime())
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

void LogEntry::writeSettings(Json::Value &root) const
{
  root["message"] = m_message.toStdString();
  root["moleQueueId"] = idTypeToJson(m_moleQueueId);
  root["entryType"] = static_cast<int>(m_entryType);
  root["time"] = m_timeStamp.toString().toStdString();
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
