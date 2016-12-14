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

#include "jobsubmissioninfo.h"
#include "messagehandler.h"

#include <QtXmlPatterns/QXmlQuery>
#include <QtCore/QRegExp>

namespace MoleQueue {
namespace Uit {

JobSubmissionInfo::JobSubmissionInfo()
  : m_valid(false), m_jobNumber(-1)
{

}

JobSubmissionInfo::JobSubmissionInfo(const JobSubmissionInfo &other)
  : m_valid(other.isValid()), m_jobNumber(other.jobNumber()),
    m_stdout(other.stdout()), m_stderr(other.stderr())
{

}

JobSubmissionInfo &JobSubmissionInfo::operator=(const JobSubmissionInfo &other)
{
  if (this != &other) {
   m_valid = other.isValid();
   m_jobNumber = other.jobNumber();
   m_stdout = other.stdout();
   m_stderr = other.stderr();
 }

  return *this;
}

bool JobSubmissionInfo::isValid() const
{
  return m_valid;
}

qint64 JobSubmissionInfo::jobNumber() const
{
  return m_jobNumber;
}

QString JobSubmissionInfo::stdout() const
{
  return m_stdout;
}

QString JobSubmissionInfo::stderr() const
{
  return m_stderr;
}

JobSubmissionInfo JobSubmissionInfo::fromXml(const QString &xml)
{
  JobSubmissionInfo info;
  info.setContent(xml);

  return info;
}


void JobSubmissionInfo::setContent(const QString &content)
{
  m_xml = content;
  m_valid = true;

  MessageHandler handler;
  QXmlQuery query;
  query.setMessageHandler(&handler);
  m_valid = query.setFocus(m_xml);

  if (!m_valid)
    return;

  query.setQuery("/JobSubmissionInfo/jobNumber/string()");
  QString jobNum;
  m_valid = query.evaluateTo(&jobNum);

  if (!m_valid)
    return;

  // jobNumber is of the form <job number>.sdb, so parse out number.
  QRegExp regex ("^(\\d+)\\..*$");
  int index = regex.indexIn(jobNum.trimmed());

  if (index != -1)
    m_jobNumber = regex.cap(1).toLongLong();

  query.setQuery("/JobSubmissionInfo/stdout/string()");
  QString out;
  m_valid = query.evaluateTo(&out);

  if (!m_valid)
    return;

  m_stdout = out.trimmed();

  query.setQuery("/JobSubmissionInfo/stderr/string()");
  QString err;
  m_valid = query.evaluateTo(&err);

  if (!m_valid)
    return;

  m_stderr = err.trimmed();
}

QString JobSubmissionInfo::xml() const
{
  return m_xml;
}

} /* namespace Uit */
} /* namespace MoleQueue */
