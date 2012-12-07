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
#include "transport/message.h"

#include "dummyconnection.h"
#include "referencestring.h"

#include "idtypeutils.h"

#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>

#include <QtCore/QCoreApplication>

using MoleQueue::Message;

class MessageTest : public QObject
{
  Q_OBJECT

private:
  DummyConnection m_conn;

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  // Test simple set/get etc
  void sanityCheck();

  // Test public methods
  void toJson(); // This indirectly (and more easily) tests toJsonObject().
  void send();
  void generateResponse();
  void generateErrorResponse();
  // Round trip reference messages
  void parse_data();
  void parse();

  // Check general message format error handling
  void parseErrorHandling();

  // Test handling of message-type specific errors. Note that any message
  // identified as a notification or response cannot fail.
  // Calling parse() a malformed error message will not return false, but the
  // message will be replaced with a server error message (with the original
  // message in the error.data.origMessage member).
  void interpretRequest();
  void interpretError();
};

void MessageTest::initTestCase()
{
}

void MessageTest::cleanupTestCase()
{
}

void MessageTest::init()
{
}

void MessageTest::cleanup()
{
}

void MessageTest::sanityCheck()
{
  // type
  Message invalid;
  QCOMPARE(invalid.type(), Message::Invalid);
  Message request(Message::Request);
  QCOMPARE(request.type(), Message::Request);
  Message notification(Message::Notification);
  QCOMPARE(notification.type(), Message::Notification);
  Message response(Message::Response);
  QCOMPARE(response.type(), Message::Response);
  Message error(Message::Error);
  QCOMPARE(error.type(), Message::Error);

  // method
  request.setMethod("Test");
  QCOMPARE(request.method(), QString("Test"));

  // params
  QJsonObject paramsObject;
  paramsObject.insert("test", QLatin1String("value"));
  request.setParams(paramsObject);
  QCOMPARE(request.params().toObject(), paramsObject);

  QJsonArray paramsArray;
  paramsArray.append(QString("Test"));
  request.setParams(paramsArray);
  QCOMPARE(request.params().toArray(), paramsArray);

  // result
  response.setResult(true);
  QCOMPARE(response.result().toBool(false), true);

  // errorCode
  int testInt = 867-5309;
  error.setErrorCode(testInt);
  QCOMPARE(error.errorCode(), testInt);

  // errorMessage
  QString testMessage = "Test Error Message";
  error.setErrorMessage(testMessage);
  QCOMPARE(error.errorMessage(), testMessage);

  // errorData
  error.setErrorData(false);
  QCOMPARE(error.errorData().toBool(true), false);

  // id
  MoleQueue::MessageIdType id(QString("IDIDIDID"));
  error.setId(id);
  QCOMPARE(error.id(), id);

  // connection
  error.setConnection(&m_conn);
  QCOMPARE(error.connection(), &m_conn);

  // endpoint
  MoleQueue::EndpointIdType endpoint("I'm an endpoint!!");
  error.setEndpoint(endpoint);
  QCOMPARE(error.endpoint(), endpoint);
}

void MessageTest::toJson()
{
  // Misc objects used in testing:
  QJsonObject testObject;
  testObject.insert("test", QLatin1String("value"));

  QJsonArray testArray;
  testArray.append(QString("Test"));

  QJsonArray testCompositeArray;
  testCompositeArray.append(MoleQueue::idTypeToJson(MoleQueue::InvalidId));
  testCompositeArray.append(testObject);
  testCompositeArray.append(testArray);
  testCompositeArray.append(true);
  testCompositeArray.append(5);
  testCompositeArray.append(5.36893473232); // This will be truncated to %.5f!
  testCompositeArray.append(QString("Abrakadabra"));

  // Test that the idtypeutils is working as expected.
  QVERIFY(testCompositeArray.first().isNull());
  QCOMPARE(MoleQueue::toIdType(testCompositeArray.first()),
           MoleQueue::InvalidId);

  // Invalid message
  Message invalid;
  QCOMPARE(QString(invalid.toJson()),
           QString(ReferenceString("message-ref/invalidJson.json")));

  // Request -- no params
  Message request(Message::Request);
  request.setMethod("testMethod");
  request.setId(MoleQueue::MessageIdType(1));
  QCOMPARE(QString(request.toJson()),
           QString(ReferenceString("message-ref/requestJson-noParams.json")));

  // Request -- object params
  request.setParams(testObject);
  QCOMPARE(QString(request.toJson()),
           QString(ReferenceString("message-ref/requestJson-objectParams.json")));

  // Request -- array params
  request.setParams(testArray);
  QCOMPARE(QString(request.toJson()),
           QString(ReferenceString("message-ref/requestJson-arrayParams.json")));

  // Notification -- no params
  Message notification(Message::Notification);
  notification.setMethod("poke");
  QCOMPARE(QString(notification.toJson()),
           QString(ReferenceString("message-ref/notificationJson-noParams.json")));

  // Notification -- object params
  notification.setParams(testObject);
  QCOMPARE(QString(notification.toJson()),
           QString(ReferenceString("message-ref/notificationJson-objectParams.json")));

  // Notification -- array params
  notification.setParams(testArray);
  QCOMPARE(QString(notification.toJson()),
           QString(ReferenceString("message-ref/notificationJson-arrayParams.json")));

  // Response
  Message response(Message::Response);
  response.setId(MoleQueue::MessageIdType(42));
  response.setMethod("Won't be in JSON string for response.");
  response.setResult(testCompositeArray);
  QCOMPARE(QString(response.toJson()),
           QString(ReferenceString("message-ref/responseJson.json")));

  // Error -- no data
  Message error(Message::Error);
  error.setId(MoleQueue::MessageIdType(13));
  error.setMethod("Won't be in JSON string for error.");
  error.setErrorCode(666);
  error.setErrorMessage("Server is possessed.");
  QCOMPARE(QString(error.toJson()),
           QString(ReferenceString("message-ref/errorJson-noData.json")));

  // Error -- primitive data
  error.setErrorData(55);
  QCOMPARE(QString(error.toJson()),
           QString(ReferenceString("message-ref/errorJson-primData.json")));

  // Error -- object data
  error.setErrorData(testObject);
  QCOMPARE(QString(error.toJson()),
           QString(ReferenceString("message-ref/errorJson-objectData.json")));

  // Error -- array data
  error.setErrorData(testArray);
  QCOMPARE(QString(error.toJson()),
           QString(ReferenceString("message-ref/errorJson-arrayData.json")));
}

void MessageTest::send()
{
  QCOMPARE(m_conn.messageCount(), 0);

  // Invalid message, no connection set
  Message invalidMessage;
  QCOMPARE(invalidMessage.send(), false);
  QCOMPARE(m_conn.messageCount(), 0);

  // Invalid message, no connection set
  invalidMessage.setConnection(&m_conn);
  QCOMPARE(invalidMessage.send(), false);
  QCOMPARE(m_conn.messageCount(), 0);

  // Valid message, no connection set
  Message request(Message::Request);
  request.setMethod("testMethod");
  QCOMPARE(request.send(), false);
  QCOMPARE(m_conn.messageCount(), 0);

  // Test id generation for requests
  request.setConnection(&m_conn);
  QVERIFY(request.id().isNull());
  QCOMPARE(request.send(), true);
  QVERIFY(!request.id().isNull());
  QCOMPARE(m_conn.messageCount(), 1);

  // Id should match the message received by the connection:
  Message connMessage = m_conn.popMessage();
  MoleQueue::MessageIdType requestId = request.id();
  QCOMPARE(requestId, connMessage.id());

  // Resending the request should assign a different id.
  QCOMPARE(request.send(), true);
  QVERIFY(!request.id().isNull());
  QCOMPARE(m_conn.messageCount(), 1);

  // The new id should not match the old one:
  connMessage = m_conn.popMessage();
  QVERIFY(requestId != connMessage.id());

  // Sending any other type of message should not modify the ids.
  MoleQueue::MessageIdType testId(QLatin1String("testId"));

  // Notifications
  // (no id testing -- ids are not used.)
  Message notification(Message::Notification, &m_conn);
  notification.setMethod("testMethod");
  QCOMPARE(notification.send(), true);
  QCOMPARE(m_conn.messageCount(), 1);
  m_conn.popMessage();

  // Response
  Message response(Message::Response, &m_conn);
  response.setId(testId);
  response.setMethod("testMethod");
  QCOMPARE(response.send(), true);
  QCOMPARE(m_conn.messageCount(), 1);
  QCOMPARE(m_conn.popMessage().id(), testId);

  // Error
  Message error(Message::Error, &m_conn);
  error.setId(testId);
  error.setErrorCode(2);
  error.setErrorMessage("Test error");
  QCOMPARE(error.send(), true);
  QCOMPARE(m_conn.messageCount(), 1);
  QCOMPARE(m_conn.popMessage().id(), testId);
}

void MessageTest::generateResponse()
{
  Message request(Message::Request, &m_conn, MoleQueue::EndpointIdType("erg"));
  request.setMethod("testMethod");
  request.setId(MoleQueue::MessageIdType(QLatin1String("testId")));

  Message response = request.generateResponse();
  QCOMPARE(response.type(), Message::Response);
  QCOMPARE(request.connection(), response.connection());
  QCOMPARE(request.endpoint(), response.endpoint());
  QCOMPARE(request.method(), response.method());
  QCOMPARE(request.id(), response.id());
}

void MessageTest::generateErrorResponse()
{
  Message request(Message::Request, &m_conn, MoleQueue::EndpointIdType("erg"));
  request.setMethod("testMethod");
  request.setId(MoleQueue::MessageIdType(QLatin1String("testId")));

  Message error = request.generateErrorResponse();
  QCOMPARE(error.type(), Message::Error);
  QCOMPARE(request.connection(), error.connection());
  QCOMPARE(request.endpoint(), error.endpoint());
  QCOMPARE(request.method(), error.method());
  QCOMPARE(request.id(), error.id());
}

void MessageTest::parse_data()
{
  QTest::addColumn<QString>("filename");

  // Test the parser by round tripping the reference sets.
  QTest::newRow("errorJson-arrayData.json") << "errorJson-arrayData.json";
  QTest::newRow("errorJson-primData.json") << "errorJson-primData.json";
  QTest::newRow("notificationJson-noParams.json") << "notificationJson-noParams.json";
  QTest::newRow("requestJson-noParams.json") << "requestJson-noParams.json";
  QTest::newRow("errorJson-noData.json") << "errorJson-noData.json";
  QTest::newRow("notificationJson-objectParams.json") << "notificationJson-objectParams.json";
  QTest::newRow("requestJson-objectParams.json") << "requestJson-objectParams.json";
  QTest::newRow("errorJson-objectData.json") << "errorJson-objectData.json";
  QTest::newRow("notificationJson-arrayParams.json") << "notificationJson-arrayParams.json";
  QTest::newRow("requestJson-arrayParams.json") << "requestJson-arrayParams.json";
  QTest::newRow("responseJson.json") << "responseJson.json";
}

void MessageTest::parse()
{
  // Fetch the current filename and load it into a ReferenceString
  QFETCH(QString, filename);
  filename.prepend("message-ref/");
  ReferenceString refStr(filename);

  // Parse the doc and create a message
  QJsonDocument doc = QJsonDocument::fromJson(refStr.toString().toLatin1());
  QVERIFY(doc.isObject());
  QJsonObject refObj = doc.object();
  QJsonValue origId;

  // Fixup the ids so that the MessageIdManager can resolve them.
  if (refObj.contains("id")) {
    Message request(Message::Request, &m_conn);
    request.setMethod("testMethod");
    QVERIFY(request.send());
    m_conn.popMessage();
    origId = refObj.value("id");
    refObj["id"] = request.id();
  }

  // Parse the message
  Message message(refObj);
  QVERIFY(message.parse());

  // Reset the id if needed
  if (!origId.isNull())
    message.setId(origId);

  // Compare strings
  QCOMPARE(refStr.toString(), QString(message.toJson()));
}

void MessageTest::parseErrorHandling()
{
  // If the message isn't raw, we should return true -- nothing to parse!
  QVERIFY(Message(Message::Request).parse());
  QVERIFY(Message(Message::Notification).parse());
  QVERIFY(Message(Message::Response).parse());
  QVERIFY(Message(Message::Error).parse());
  QVERIFY(Message(Message::Invalid).parse());

  // Construct a valid object and verify that it parses.
  QJsonObject validObj;
  validObj.insert("jsonrpc", QLatin1String("2.0"));
  validObj.insert("id", QLatin1String("5"));
  validObj.insert("method", QLatin1String("testMethod"));
  QVERIFY(Message(validObj).parse());

  // This will be our test object.
  QJsonObject obj;

  // Must contain 'jsonrpc' member
  obj = validObj;
  obj.remove("jsonrpc");
  QVERIFY(!Message(obj).parse());

  // 'jsonrpc' member must be a string
  obj = validObj;
  obj["jsonrpc"] = 2.0;
  QVERIFY(!Message(obj).parse());

  // 'jsonrpc' member must be exactly "2.0"
  obj = validObj;
  obj["jsonrpc"] = QString("1.0 + 1.0");
  QVERIFY(!Message(obj).parse());

  // Must have either id or method
  obj = validObj;
  obj.remove("id");
  obj.remove("method");
  QVERIFY(!Message(obj).parse());

  // If present, method must be a string
  obj = validObj;
  obj["method"] = true;
  QVERIFY(!Message(obj).parse());
}

void MessageTest::interpretRequest()
{
  // Construct a valid object and verify that it parses.
  QJsonObject validObj;
  validObj.insert("jsonrpc", QLatin1String("2.0"));
  validObj.insert("id", QLatin1String("5"));
  validObj.insert("method", QLatin1String("testMethod"));
  QVERIFY(Message(validObj).parse());

  // This will be our test object.
  QJsonObject obj;

  // If params is present, it must be a structured type (i.e. array or object)
  obj = validObj;
  obj["params"] = true;
  QVERIFY(!Message(obj).parse());
}

// Register the id, attempt to parse, and check that the parsed error object
// shows a server error occurred (if serverErr is true).
#define TEST_ERROR_PARSING(obj, serverErr) \
{ \
  if (obj.contains("id")) { \
    Message dummyRequest(Message::Request, &m_conn); \
    dummyRequest.setMethod("testMethod"); \
    QVERIFY(dummyRequest.send()); \
    m_conn.popMessage(); \
    obj["id"] = dummyRequest.id(); \
  } \
  Message msg(obj); \
  QVERIFY(msg.parse()); \
  QCOMPARE(msg.type(), Message::Error); \
  if (serverErr) \
    QCOMPARE(msg.errorCode(), -32000); \
  else \
    QVERIFY(msg.errorCode() != -32000); \
}

void MessageTest::interpretError()
{
  // If the error is malformed, parsing will NOT fail, as we cannot send an
  // error reply. Instead, the error metadata is replaced with a server error
  // (code = -32000)

  // Construct a valid object and verify that it parses.
  QJsonObject validErrorObj;
  validErrorObj.insert("code", 2);
  validErrorObj.insert("message", QString("Error message"));
  validErrorObj.insert("data", 5);
  QJsonObject validObj;
  validObj.insert("jsonrpc", QLatin1String("2.0"));
  validObj.insert("id", QLatin1String("5"));
  validObj.insert("error", validErrorObj);
  TEST_ERROR_PARSING(validObj, false);

  // This will be our test object.
  QJsonObject obj;
  QJsonObject errorObj;

  // error must be an object
  obj = validObj;
  obj["error"] = 5;
  TEST_ERROR_PARSING(obj, true);

  // error.code must be present
  errorObj = validErrorObj;
  errorObj.remove("code");
  obj = validObj;
  obj["error"] = errorObj;
  TEST_ERROR_PARSING(obj, true);

  // error.code must be numeric
  errorObj = validErrorObj;
  errorObj["code"] = true;
  obj = validObj;
  obj["error"] = errorObj;
  TEST_ERROR_PARSING(obj, true);

  // error.code must be integral
  errorObj = validErrorObj;
  errorObj["code"] = 2.3;
  obj = validObj;
  obj["error"] = errorObj;
  TEST_ERROR_PARSING(obj, true);

  // error.message must be present
  errorObj = validErrorObj;
  errorObj.remove("message");
  obj = validObj;
  obj["error"] = errorObj;
  TEST_ERROR_PARSING(obj, true);

  // error.message must be a string
  errorObj = validErrorObj;
  errorObj["message"] = 2.66;
  obj = validObj;
  obj["error"] = errorObj;
  TEST_ERROR_PARSING(obj, true);
}

QTEST_MAIN(MessageTest)

#include "messagetest.moc"
