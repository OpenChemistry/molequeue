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

#include "queues/uit/authenticatecont.h"
#include "queues/uit/authenticateresponse.h"
#include "referencestring.h"
#include "xmlutils.h"

class AuthenticateContTest : public QObject
{
  Q_OBJECT
private slots:
  void testToXmlNoReply();
  void testToXmlWithReply();
};

void AuthenticateContTest::testToXmlNoReply()
{
  QList<MoleQueue::Uit::Prompt> prompts;
  prompts << MoleQueue::Uit::Prompt(0, "prompt1");
  prompts << MoleQueue::Uit::Prompt(1, "prompt2");

  QString id = "sessionId";

  MoleQueue::Uit::AuthenticateCont authCont(id, prompts);

  ReferenceString expected(
    "authenticatecont-ref/authenticatecont-no-reply.xml");
  QCOMPARE(authCont.toXml(), XmlUtils::stripWhitespace(expected));

}

void AuthenticateContTest::testToXmlWithReply()
{
  QList<MoleQueue::Uit::Prompt> prompts;
  MoleQueue::Uit::Prompt prompt1(0, "prompt1");
  prompt1.setUserResponse("reply1");
  MoleQueue::Uit::Prompt prompt2(1, "prompt2");
  prompt2.setUserResponse("reply2");

  prompts << prompt1 << prompt2;

  QString id = "sessionId";

  MoleQueue::Uit::AuthenticateCont authCont(id, prompts);

  ReferenceString expected(
    "authenticatecont-ref/authenticatecont-reply.xml");
  QCOMPARE(authCont.toXml(), XmlUtils::stripWhitespace(expected));

}

QTEST_MAIN(AuthenticateContTest)

#include "authenticateconttest.moc"
