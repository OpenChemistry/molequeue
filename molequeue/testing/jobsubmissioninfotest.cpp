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

#include "queues/uit/jobsubmissioninfo.h"
#include "referencestring.h"
#include "xmlutils.h"

class JobSubmissionInfoTest : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();

  void testFromXml();
  void testJobNumberRegex();

private:
  QString m_jobSubmissionInfoXml;
};

void JobSubmissionInfoTest::initTestCase()
{
m_jobSubmissionInfoXml
    = XmlUtils::stripWhitespace(
        ReferenceString("jobsubmissioninfo-ref/jobsubmissioninfo.xml"));
}

void JobSubmissionInfoTest::testFromXml()
{
  MoleQueue::Uit::JobSubmissionInfo info =
    MoleQueue::Uit::JobSubmissionInfo::fromXml(m_jobSubmissionInfoXml);

  QVERIFY(info.isValid());
  QVERIFY(info.jobNumber() == 343242);
  QCOMPARE(info.stdout(),
           QString("Job &lt;75899&gt; is submitted to debug queue."));
  QCOMPARE(info.stderr(), QString("error"));
}

void JobSubmissionInfoTest::testJobNumberRegex()
{
  QString testJobString = "234234.sdb\n";

  QRegExp parser ("^(\\d+)\\.sdb$");
  int index = parser.indexIn(testJobString.trimmed());

  QVERIFY(index != -1);
  QCOMPARE(parser.cap(1), QString("234234"));

}

QTEST_MAIN(JobSubmissionInfoTest)

#include "jobsubmissioninfotest.moc"
