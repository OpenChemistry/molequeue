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

#include "job.h"

namespace MoleQueue
{

JobObject::JobObject()
{
}

JobObject::~JobObject()
{
}

void JobObject::setValue(const QString &key, const QVariant &value_)
{
  Json::Value result(Json::objectValue);

  switch(value_.type()) {
  case QVariant::Bool:
    result = value_.toBool();
    break;
  case QVariant::Int:
    result = value_.toInt();
    break;
  case QVariant::LongLong:
    result = value_.toLongLong();
    break;
  case QVariant::UInt:
    result = value_.toUInt();
    break;
  case QVariant::ULongLong:
    result = value_.toULongLong();
    break;
  case QVariant::Double:
    result = value_.toDouble();
    break;
  case QVariant::String:
    result = value_.toString().toStdString();
    break;
  case QVariant::ByteArray:
    result = value_.toByteArray().constData();
    break;
  default:
    result = Json::nullValue;
  }

  m_value[key.toStdString()] = result;
}

QVariant JobObject::value(const QString &key) const
{
  // Extract option data
  Json::Value result = m_value[std::string(key.toAscii())];

  switch (result.type()) {
  case Json::nullValue:
    return QVariant();
  case Json::intValue:
    return result.asLargestInt();
  case Json::uintValue:
    return result.asLargestUInt();
  case Json::realValue:
    return result.asDouble();
  case Json::stringValue:
    return result.asCString();
  case Json::booleanValue:
    return result.asBool();
  default:
    return QVariant();
  }
}

} // End namespace MoleQueue
