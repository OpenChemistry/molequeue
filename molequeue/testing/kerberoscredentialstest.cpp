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

#include "queues/uit/kerberoscredentials.h"
#include "referencestring.h"
#include "xmlutils.h"

class KerberosCredentialsTest : public QObject
{
  Q_OBJECT
private slots:
  void testToXml();

};

void KerberosCredentialsTest::testToXml()
{
  ReferenceString expected("kerberoscredentials-ref/kerberoscredentials.xml");

  MoleQueue::KerberosCredentials credentials("test", "test");
  QCOMPARE(credentials.toXml(),
           XmlUtils::stripWhitespace(expected));
}

QTEST_MAIN(KerberosCredentialsTest)

#include "kerberoscredentialstest.moc"
