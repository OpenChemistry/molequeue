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

#ifndef QTJSON_H
#define QTJSON_H

#include <json/json-forwards.h>

#include <QtCore/QVariant>

namespace QtJson
{
/// Convert a QVariant into a JSON Value. If the Variant is a string-keyed hash,
/// a JSON dictionary object is returned. For a QList, a json array is returned.
/// Other simple types (and members of the mentioned containers) will be
/// converted to their corresponding json types.
/// @param variant input QVariant.
/// @return Json value.
Json::Value toJson(const QVariant &variant);
/// Convert a JSON Value to a QVariant. If the value is a dictionary object, a
/// QVariantHash is returned. If the value is an array, a QList is returned.
/// Other simple types (and members of the mentioned containers) are interpreted
/// and wrapped in a QVariant.
/// @param value Json value.
/// @return input hash.
QVariant toVariant(const Json::Value &value);
}

#endif // QTJSON_H
