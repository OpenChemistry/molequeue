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

#include "queues/uit/userhostassoclist.h"
#include "referencestring.h"
#include "xmlutils.h"

class UserHostAssocListTest : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();

  void testFromXml();

private:
  QString m_userHostAssocXml;
};

void UserHostAssocListTest::initTestCase()
{
  m_userHostAssocXml
    = XmlUtils::stripWhitespace(
        ReferenceString("userhostassoclist-ref/userhostassoclisttest.xml"));
}

void UserHostAssocListTest::testFromXml()
{
  MoleQueue::Uit::UserHostAssocList userHostAssocList =
    MoleQueue::Uit::UserHostAssocList::fromXml(m_userHostAssocXml);

  QCOMPARE(userHostAssocList.userHostAssocs().size(), 2);

  MoleQueue::Uit::UserHostAssoc userHostAssoc
    = userHostAssocList.userHostAssocs()[0];

  QCOMPARE(userHostAssoc.account(), QString("user"));
  QVERIFY(userHostAssoc.hostId() == 2);
  QCOMPARE(userHostAssoc.transportMethod(), QString("MSRC_KERBEROS"));
  QCOMPARE(userHostAssoc.hostName(), QString("ruby.erdc.hpc.mil"));
  QCOMPARE(userHostAssoc.description(), QString("Origin 3900"));

  userHostAssoc = userHostAssocList.userHostAssocs()[1];

  QCOMPARE(userHostAssoc.account(), QString("user"));
  QVERIFY(userHostAssoc.hostId() == 3);
  QCOMPARE(userHostAssoc.transportMethod(), QString("MSRC_KERBEROS"));
  QCOMPARE(userHostAssoc.hostName(), QString("lead.erdc.hpc.mil"));
  QCOMPARE(userHostAssoc.description(), QString("Origin 3901"));
  QCOMPARE(userHostAssoc.systemName(), QString("ERDC::DIAMOND"));
}


QTEST_MAIN(UserHostAssocListTest)

#include "userhostassoclisttest.moc"
