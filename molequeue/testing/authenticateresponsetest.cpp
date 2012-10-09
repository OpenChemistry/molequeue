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

#include "queues/uit/authenticateresponse.h"
#include "referencestring.h"
#include "xmlutils.h"

class AuthenticateResponseTest : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();

  void testFromXml();
  void testXpathExpressions();

private:
  QString m_authenticateResponseXml;
};

void AuthenticateResponseTest::initTestCase()
{
  m_authenticateResponseXml
    = XmlUtils::stripWhitespace(
        ReferenceString("authenticateresponse-ref/authenticateresponse.xml"));
}

void AuthenticateResponseTest::testFromXml()
{
  MoleQueue::AuthenticateResponse response =
    MoleQueue::AuthenticateResponse::fromXml(m_authenticateResponseXml);

  QVERIFY(response.hasPrompts());
  QCOMPARE(response.authSessionId(),
           QString("FE09938C-84BC-E75A-D767-84B85F48C4DB"));
  QCOMPARE(response.errorMessage(), QString("error"));

  QCOMPARE(response.prompts().size(), 2);

  QCOMPARE(response.prompts()[0].id(), 0);
  QCOMPARE(response.prompts()[0].prompt(), QString("SecurID Passcode"));

  QCOMPARE(response.prompts()[1].id(), 2);
  QCOMPARE(response.prompts()[1].prompt(), QString("Password"));
}

void AuthenticateResponseTest::testXpathExpressions()
{
  QXmlQuery query;
  query.setFocus(m_authenticateResponseXml);
  query.setQuery("/AuthenticateResponse/success/string()");
  QString result;
  query.evaluateTo(&result);
  QCOMPARE(result.trimmed(), QString("false"));

  QStringList expectedIds;
  expectedIds << "0" << "2";

  QStringList expectedPrompts;
  expectedPrompts << "SecurID Passcode" << "Password";

  QStringList ids;
  query.setQuery("/AuthenticateResponse/prompts/Prompt/id/string()");
  int index = 0;
  foreach (const QString &id, ids) {

    QCOMPARE(id, expectedIds[index]);

    query.bindVariable("id", QVariant(id));
    query.setQuery(
      QString("/AuthenticateResponse/prompts/Prompt[id=$id]/prompt/string()"));

    QString prompt;
    query.evaluateTo(&prompt);

    QCOMPARE(prompt, expectedPrompts[index++]);
  }
}

QTEST_MAIN(AuthenticateResponseTest)

#include "authenticateresponsetest.moc"
