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
#include <QtCore/QFileInfo>

namespace MoleQueue {

Program::Program(Queue *parentQueue) :
  QObject(parentQueue),
  m_name("Unknown"),
  m_runDirect(true),
  m_delimiter("$$"),
  m_queue(parentQueue)
{
}

Program::Program(const Program &other)
  : QObject(other.parent()),
    m_name(other.m_name),
    m_title(other.m_title),
    m_runDirect(other.m_runDirect),
    m_runTemplate(other.m_runTemplate),
    m_delimiter(other.m_delimiter),
    m_queue(other.m_queue)
{
}

Program::~Program()
{
}

QString Program::queueName() const
{
  if (m_queue)
    return m_queue->name();
  else
    return "None";
}

} // End namespace
