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
#include <QtCore/QList>

#include "queues/uit/dirlistinginfo.h"
#include "referencestring.h"
#include "xmlutils.h"




class DirListingInfoTest : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();

  void testFromXml();

private:
  QString m_dirListingInfo;
};

void DirListingInfoTest::initTestCase()
{
  m_dirListingInfo
    = ReferenceString("dirlistinginfo-ref/dirlistinginfo.xml");
}

void DirListingInfoTest::testFromXml()
{
  MoleQueue::Uit::DirListingInfo list
    = MoleQueue::Uit::DirListingInfo::fromXml(m_dirListingInfo);

  QCOMPARE(list.directories().size(), 2);
  QCOMPARE(list.files().size(), 2);
  QCOMPARE(list.currentDirectory(), QString("/home/u/username"));
  MoleQueue::Uit::FileInfo info = list.directories()[0];

  QVERIFY(info.size() == 4096);
  QCOMPARE(info.name(), QString("."));
  QCOMPARE(info.perms(), QString("drwx------"));
  QCOMPARE(info.date(), QString("Sep 19 10:51"));
  QCOMPARE(info.user(), QString("username"));
  QCOMPARE(info.group(), QString("chl"));

  info = list.directories()[1];

  QVERIFY(info.size() == 4096);
  QCOMPARE(info.name(), QString("test"));
  QCOMPARE(info.perms(), QString("drwx------"));
  QCOMPARE(info.date(), QString("Sep 19 10:51"));
  QCOMPARE(info.user(), QString("username"));
  QCOMPARE(info.group(), QString("chl"));

  info = list.files()[0];

  QVERIFY(info.size() == 1705);
  QCOMPARE(info.name(), QString(".bash_history"));
  QCOMPARE(info.perms(), QString("-rw-------"));
  QCOMPARE(info.date(), QString("Sep 6 16:44"));
  QCOMPARE(info.user(), QString("username"));
  QCOMPARE(info.group(), QString("chl"));

  info = list.files()[1];

  QVERIFY(info.size() == 1705);
  QCOMPARE(info.name(), QString("file"));
  QCOMPARE(info.perms(), QString("-rw-------"));
  QCOMPARE(info.date(), QString("Sep 6 16:44"));
  QCOMPARE(info.user(), QString("username"));
  QCOMPARE(info.group(), QString("chl"));
}


QTEST_MAIN(DirListingInfoTest)

#include "dirlistinginfotest.moc"
