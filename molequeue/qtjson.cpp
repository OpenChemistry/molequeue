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

#include "qtjson.h"

#include <json/json.h>

namespace QtJson
{
Json::Value qtToJson(const QVariant &variant)
{
  switch (variant.type()) {
  case QVariant::List: {
    QVariantList list = variant.toList();
    Json::Value result(Json::arrayValue);
    foreach (const QVariant &var, list) {
      result.append(qtToJson(var));
    }
    return result;
  }
  case QVariant::Hash: {
    QVariantHash hash = variant.toHash();
    Json::Value result(Json::objectValue);
    for (QVariantHash::const_iterator it = hash.constBegin(),
         it_end = hash.constEnd(); it != it_end; ++it) {
      const std::string name = it.key().toStdString();
      result[name] = qtToJson(it.value());
    }
    return result;
  }
  case QVariant::Bool:
    return Json::Value(variant.toBool());
  case QVariant::Int:
    return Json::Value(variant.toInt());
  case QVariant::LongLong:
    return Json::Value(variant.toLongLong());
  case QVariant::UInt:
    return Json::Value(variant.toUInt());
  case QVariant::ULongLong:
    return Json::Value(variant.toULongLong());
  case QVariant::Double:
    return Json::Value(variant.toDouble());
  case QVariant::String:
    return Json::Value(variant.toString().toStdString());
  case QVariant::ByteArray:
    return Json::Value(variant.toByteArray().constData());
  default:
    return Json::Value(Json::nullValue);
  }
}

QVariant jsonToQt(const Json::Value &value)
{
  switch (value.type()) {
  case Json::arrayValue: {
    QVariantList result;
    result.reserve(value.size());
    for (Json::Value::const_iterator it = value.begin(),
         it_end = value.end(); it != it_end; ++it) {
      result.append(jsonToQt(*it));
    }
    return result;
  }
  case Json::objectValue: {
    QVariantHash result;
    result.reserve(value.size());
    for (Json::Value::const_iterator it = value.begin(),
         it_end = value.end(); it != it_end; ++it) {
      result.insert(QString(it.memberName()), jsonToQt(*it));
    }
    return result;
  }
  case Json::intValue:
    return QVariant(value.asLargestInt());
  case Json::uintValue:
    return QVariant(value.asLargestUInt());
  case Json::realValue:
    return QVariant(value.asDouble());
  case Json::stringValue:
    return QVariant(QString(value.asCString()));
  case Json::booleanValue:
    return QVariant(value.asBool());
  default:
    return QVariant();
  }
}

} // end namespace QtJson
