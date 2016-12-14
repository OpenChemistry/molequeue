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

#include "jobeventlist.h"
#include "messagehandler.h"

#include <QtXmlPatterns/QAbstractXmlReceiver>
#include <QtXmlPatterns/QXmlNamePool>
#include <QtXmlPatterns/QXmlQuery>
#include <QtCore/QList>
#include <QtCore/QRegExp>

namespace {

/**
 * @brief XML receiver to parser JobEvent tags.
 */
class JobEventListXmlReceiver : public QAbstractXmlReceiver
{

public:
  JobEventListXmlReceiver(QXmlNamePool pool);

  void  atomicValue ( const QVariant & value ) { Q_UNUSED(value); }
  void  attribute ( const QXmlName & name, const QStringRef & value ) {
    Q_UNUSED(name);
    Q_UNUSED(value);
  }
  void  characters ( const QStringRef & value );
  void  comment ( const QString & value ) { Q_UNUSED(value); }
  void  endDocument () {}
  void  endElement ();
  void  endOfSequence () {};
  void  namespaceBinding ( const QXmlName & name ) { Q_UNUSED(name); };
  void  processingInstruction ( const QXmlName & target,
                                const QString & value) {
    Q_UNUSED(target);
    Q_UNUSED(value);
  };
  void  startDocument () {};
  void  startElement ( const QXmlName & name );
  void  startOfSequence () {};

  QList<MoleQueue::Uit::JobEvent> jobEvents() const {
    return m_events;
  };

private:
  QXmlNamePool m_pool;
  MoleQueue::Uit::JobEvent m_currentEvent;
  QString m_currentName;
  QString m_currentValue;
  QList<MoleQueue::Uit::JobEvent> m_events;
  int tagDepth;

};

JobEventListXmlReceiver::JobEventListXmlReceiver(QXmlNamePool pool)
  : m_pool(pool), tagDepth(0)
{

}


void  JobEventListXmlReceiver::characters ( const QStringRef & value )
{
  if (!m_currentName.isEmpty()) {
    m_currentValue.append(value);
  }
}

void  JobEventListXmlReceiver::endElement ()
{
  if (tagDepth-- == 1) {
    // Save the current event.
    m_events.append(m_currentEvent);
    m_currentEvent = MoleQueue::Uit::JobEvent();
  }
  else if (m_currentName == "acctHost") {
    m_currentEvent.setAcctHost(m_currentValue);
  }
  else if (m_currentName == "eventType") {
    m_currentEvent.setEventType(m_currentValue);
  }
  else if (m_currentName == "eventTime") {
    m_currentEvent.setEventTime(m_currentValue.toInt());
  }
  else if (m_currentName == "jobID") {
    QRegExp regex ("^(\\d+)\\..*$");
    regex.indexIn(m_currentValue.trimmed());
    m_currentEvent.setJobId(regex.cap(1).toLongLong());
  }
  else if (m_currentName == "jobQueue") {
    m_currentEvent.setJobQueue(m_currentValue);
  }
  else if (m_currentName == "jobStatus") {
    m_currentEvent.setJobStatus(m_currentValue);
  }
  else if (m_currentName == "jobStatusText") {
    m_currentEvent.setJobStatusText(m_currentValue);
  }

  m_currentName.clear();
  m_currentValue.clear();
}

void  JobEventListXmlReceiver::startElement ( const QXmlName & name )
{
  tagDepth++;
  m_currentName =  name.localName(m_pool);
  m_currentValue.clear();
}

} /* namespace anonymous */

namespace MoleQueue {
namespace Uit {

JobEventList::JobEventList()
  : m_valid(false)
{

}

JobEventList::JobEventList(const JobEventList &other)
  : m_valid(false), m_jobEvents(other.jobEvents())
{

}

JobEventList JobEventList::fromXml(const QString &xml)
{
  QList<qint64> empty;
  return fromXml(xml, "", empty);
}

JobEventList JobEventList::fromXml(const QString &xml, const QString &userName,
                                   QList<qint64> jobIds)
{
  JobEventList list;
  list.setContent(xml, userName, jobIds);

  return list;
}

void JobEventList::setContent(const QString &content, const QString &userName,
                              QList<qint64> jobIds)
{
  m_xml = content;
  m_valid = true;

  MessageHandler handler;
  QXmlQuery query;
  query.setMessageHandler(&handler);
  JobEventListXmlReceiver receiver(query.namePool());
  query.setFocus(m_xml);

  if (jobIds.isEmpty()) {
    query.setQuery("/list/JobEvent");
  }
  else {
    QString xpath = "/list/JobEvent";
    query.setQuery(xpath);
  }

  m_valid = query.evaluateTo(&receiver);

  m_jobEvents = receiver.jobEvents();

  QMutableListIterator<JobEvent> it(m_jobEvents);
  while (it.hasNext()) {
    JobEvent event = it.next();
    if (!jobIds.contains(event.jobId())) {
      it.remove();
    }
  }
}

} /* namespace Uit */
} /* namespace MoleQueue */
