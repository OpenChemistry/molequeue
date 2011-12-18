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

#include "job.h"

#include <QFileInfo>

#include "program.h"

namespace MoleQueue {

Job::Job(const Program *program)
  : QObject(),
    m_program(program)
{
}

Job::~Job()
{
}

void Job::setName(const QString &name)
{
  m_name = name;
}

QString Job::name() const
{
  return m_name;
}

void Job::setTitle(const QString &title)
{
  m_title = title;
}

QString Job::title() const
{
  return m_title;
}

const Program* Job::program() const
{
  return m_program;
}

const Queue* Job::queue() const
{
  return m_program->queue();
}

void Job::setWorkingDirectory(const QString &dir)
{
  m_workingDirectory = dir;
  setReplacement("workingDirectory", dir);
}

void Job::setInputFile(const QString &file)
{
  m_inputFile = file;
  QFileInfo info(file);
  setReplacement("input", info.baseName());
}

void Job::setOutputFile(const QString &file)
{
  m_outputFile = file;
  QFileInfo info(file);
  setReplacement("output", info.baseName());
}

QString Job::statusString() const
{
  switch (m_status) {
  case UNDEFINED:
    return "Undefined";
  case QUEUED:
    return "Queued (L)";
  case REMOTEQUEUED:
    return "Queued (R)";
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

QString Job::replacement(const QString &keyword) const
{
  if (m_replacements.contains(keyword))
    return m_replacements[keyword];
  else
    return QString();
}

void Job::setReplacement(const QString &keyword, const QString &value)
{
  m_replacements[keyword] = value;
}

QString Job::replacementList() const
{
  QString list;
  foreach(const QString &key, m_replacements.keys()) {
    list = "Keyword: " + key + " = " + m_replacements[key] + "\n";
  }
  return list;
}

QString Job::expandedRunTemplate() const
{
  QString delimiter = m_program->delimiter();
  QString runTemplate = m_program->runTemplate();

  QString expanded(runTemplate);
  foreach(const QString &key, m_replacements.keys()) {
    expanded = expanded.replace(delimiter + key + delimiter,
                                m_replacements[key]);
  }

  return expanded;
}

} // end MoleQueue namespace
