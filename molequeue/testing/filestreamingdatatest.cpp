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

#include "queues/uit/filestreamingdata.h"
#include "referencestring.h"
#include "xmlutils.h"

class FileStreamingDataTest : public QObject
{
  Q_OBJECT
private slots:
  void testToXml();

};

void FileStreamingDataTest::testToXml()
{
  MoleQueue::Uit::FileStreamingData fileData;

  fileData.setToken("TOKENDATA");
  fileData.setFileName("/home/u/username/TESTFILE_UPLOAD");
  fileData.setHostID(1);
  fileData.setUserName("username");


  ReferenceString expected(
    "filestreamingdata-ref/filestreamingdata.xml");
  QCOMPARE(fileData.toXml(), XmlUtils::stripWhitespace(expected));

}

QTEST_MAIN(FileStreamingDataTest)

#include "filestreamingdatatest.moc"
