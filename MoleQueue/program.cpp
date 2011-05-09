/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "program.h"

#include "queue.h"

#include <QtCore/QDebug>

namespace MoleQueue {

Program::Program(Queue *queue) : m_runDirect(true), m_delimiter("$$"),
  m_status(UNDEFINED), m_queue(queue)
{
}

Program::~Program()
{
}

Program::Program(const Program &other)
{
  m_runDirect = other.m_runDirect;
  m_runTemplate = other.m_runTemplate;
  m_delimiter = other.m_delimiter;
  m_replacements = other.m_replacements;
  m_workingDirectory = other.m_workingDirectory;
  m_inputFile = other.m_inputFile;
  m_queue = other.m_queue;
  m_title = other.m_title;
  m_name = other.m_name;
}

QString Program::expandedRunTemplate() const
{
  QString expanded(m_runTemplate);
  foreach(const QString &key, m_replacements.keys()) {
    expanded = expanded.replace(m_delimiter + key + m_delimiter,
                                m_replacements[key]);
  }

  return expanded;
}

QString Program::replacement(const QString &keyword) const
{
  if (m_replacements.contains(keyword))
    return m_replacements[keyword];
  else
    return QString();
}

void Program::setReplacement(const QString &keyword, const QString &value)
{
  m_replacements[keyword] = value;
}

QString Program::replacementList() const
{
  QString list;
  foreach(const QString &key, m_replacements.keys()) {
    list = "Keyword: " + key + " = " + m_replacements[key] + "\n";
  }
  return list;
}

QString Program::queueName() const
{
  if (m_queue)
    return m_queue->name();
  else
    return "None";
}

QString Program::statusString() const
{
  switch (m_status) {
  case UNDEFINED:
    return "Undefined";
  case QUEUED:
    return "Queued locally";
  case REMOTEQUEUED:
    return "Queued remotely";
  case RUNNING:
    return "Running";
  case COMPLETE:
    return "Completed";
  case FAILED:
    return "Failed";
  default:
    return "Undefined";
  }
}

} // End namespace
