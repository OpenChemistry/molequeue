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
  m_value[key] = QJsonValue::fromVariant(value_);
}

QVariant JobObject::value(const QString &key) const
{
  return m_value[key].toVariant();
}

void JobObject::setInputFile(const QString &fileName, const QString &contents)
{
  m_value["inputFile"] = fileSpec(fileName, contents);
}

void JobObject::setInputFile(const QString &path)
{
  m_value["inputFile"] = fileSpec(path);
}

QJsonObject JobObject::fileSpec(const QString &fileName, const QString &contents)
{
  QJsonObject result;
  result["filename"] = fileName;
  result["contents"] = contents;
  return result;
}

QJsonObject JobObject::fileSpec(const QString &path)
{
  QJsonObject result;
  result["path"] = path;
  return result;
}

} // End namespace MoleQueue
