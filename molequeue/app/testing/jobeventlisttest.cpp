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

#include <QtTest>
#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QAbstractXmlReceiver>
#include <QtCore/QList>
#include "queues/queueuit.h"

#include "queues/uit/authenticateresponse.h"
#include "queues/uit/jobeventlist.h"
#include "referencestring.h"
#include "xmlutils.h"




class JobEventTest : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();

  void testFromXml();
  void testFromXmlWithJobId();
  void testFromXmlWithJobIdsUser();
  void testFromXmlWithJobIds();


private:
  QString m_jobEventXml;
};

void JobEventTest::initTestCase()
{
  m_jobEventXml = ReferenceString("jobeventlist-ref/jobeventlist.xml");
}

void JobEventTest::testFromXml()
{
  MoleQueue::Uit::JobEventList list
    = MoleQueue::Uit::JobEventList::fromXml(m_jobEventXml);

  QVERIFY(list.isValid());
  QCOMPARE(list.jobEvents().size(), 6);
}

void JobEventTest::testFromXmlWithJobId()
{

  QList<qint64> jobIds;

  jobIds << 100535;

  MoleQueue::Uit::JobEventList list
    = MoleQueue::Uit::JobEventList::fromXml(m_jobEventXml, "username", jobIds);

  QVERIFY(list.isValid());
  QCOMPARE(list.jobEvents().size(),  2);

  foreach(const MoleQueue::Uit::JobEvent &e, list.jobEvents()) {

    QCOMPARE(e.acctHost(), QString("ruby.erdc.hpc.mil"));
    QVERIFY(e.eventTime() == 1124393333);
    QCOMPARE(e.eventType(), QString("JOB_FINISH"));
    QCOMPARE(e.jobStatus(), QString("64"));
    QCOMPARE(e.jobQueue(), QString("biggiesmalls"));
    QVERIFY(e.jobId() == 100535);
    QCOMPARE(e.jobStatusText(), QString("done"));
  }
}

void JobEventTest::testFromXmlWithJobIdsUser()
{

  QList<qint64> jobIds;

  jobIds << 100535 << 100539;

  MoleQueue::Uit::JobEventList list
    = MoleQueue::Uit::JobEventList::fromXml(m_jobEventXml, "username2", jobIds);

  QVERIFY(list.isValid());
  QCOMPARE(list.jobEvents().size(),  2);

  foreach(const MoleQueue::Uit::JobEvent &e, list.jobEvents()) {

    QCOMPARE(e.acctHost(), QString("ruby.erdc.hpc.mil"));
    QVERIFY(e.eventTime() == 1124393333);
    QCOMPARE(e.eventType(), QString("JOB_FINISH"));
    QCOMPARE(e.jobStatus(), QString("64"));
    QCOMPARE(e.jobQueue(), QString("biggiesmalls2"));
    QVERIFY(e.jobId() == 100539);
    QCOMPARE(e.jobStatusText(), QString("done"));
  }
}

void JobEventTest::testFromXmlWithJobIds()
{

  QList<qint64> jobIds;

  jobIds << 100535 << 100536;

  MoleQueue::Uit::JobEventList list
    = MoleQueue::Uit::JobEventList::fromXml(m_jobEventXml, "username", jobIds);

  QVERIFY(list.isValid());
  QCOMPARE(list.jobEvents().size(),  3);
}

QTEST_MAIN(JobEventTest)

#include "jobeventlisttest.moc"
