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

#include "jsonrpc.h"

#include "job.h"
#include "program.h"
#include "queue.h"
#include "queuemanager.h"
#include "queues/local.h"
#include "queues/sge.h"
#include "transport/connection.h"

#include <json/json.h>

#include <QtTest>
#include <QtCore/QFile>

using namespace MoleQueue;

Q_DECLARE_METATYPE(Json::Value)

class JsonRpcTest : public QObject
{
  Q_OBJECT

private:
  PacketType readReferenceString(const QString &filename);
  void printNode(const Json::Value &);

  bool m_error;
  JsonRpc m_rpc;
  PacketType m_packet;
  PacketType m_refPacket;
  QueueManager m_qmanager;
  Json::Reader m_reader;
  Json::StyledWriter m_writer;
  Json::Value m_root;
  Connection *m_connection;

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  // Unit test functions go below:
  void validateRequest();
  void validateResponse();
  void validateNotification();

  void generateJobRequest();
  void generateJobSubmissionConfirmation();
  void generateErrorResponse();
  void generateJobCancellation();
  void generateJobCancellationConfirmation();
  void generateQueueListRequest();
  void generateQueueList();
  void generateJobStateChangeNotification();

  void interpretIncomingPacket_unparsable();
  void interpretIncomingPacket_invalidRequest();
  void interpretIncomingPacket_unrecognizedRequest();
  void interpretIncomingPacket_listQueuesRequest();
  void interpretIncomingPacket_listQueuesResult();
  void interpretIncomingPacket_listQueuesError();
  void interpretIncomingPacket_submitJobRequest();
  void interpretIncomingPacket_submitJobResult();
  void interpretIncomingPacket_submitJobError();
  void interpretIncomingPacket_cancelJobRequest();
  void interpretIncomingPacket_cancelJobResult();
  void interpretIncomingPacket_cancelJobError();
  void interpretIncomingPacket_jobStateChange();

};

class TestConnection : public Connection
{
  Q_OBJECT
public:
  TestConnection(QObject *parentObject = 0 ) : Connection(parentObject) {};

  void open() {};
  void start() {};
  void send(const Message msg) { Q_UNUSED(msg); };
  void close() {};
  bool isOpen() { return false; };
  QString connectionString() const { return ""; };
};


PacketType JsonRpcTest::readReferenceString(const QString &filename)
{
  QString realFilename = TESTDATADIR + filename;
  QFile refFile (realFilename);
  if (!refFile.open(QFile::ReadOnly)) {
    qDebug() << "Cannot access reference file" << realFilename;
    return "";
  }
  PacketType contents = refFile.readAll();
  refFile.close();
  return contents;
}

void JsonRpcTest::printNode(const Json::Value &root)
{
  std::string str;
  str = m_writer.write(root);
  qDebug() << str.c_str();
}

void JsonRpcTest::initTestCase()
{
//  m_rpc.setDebug(true);

  Queue *queueTmp = m_qmanager.addQueue("Some big ol' cluster",
                                        "Sun Grid Engine");
  Program *progTmp = new Program (NULL);
  progTmp->setName("Quantum Tater");
  queueTmp->addProgram(progTmp);
  progTmp = new Program (*progTmp);
  progTmp->setName("Crystal Math");
  queueTmp->addProgram(progTmp);
  progTmp = new Program (*progTmp);
  progTmp->setName("Nebulous Nucleus");
  queueTmp->addProgram(progTmp);

  queueTmp = m_qmanager.addQueue("Puny local queue", "Local");
  progTmp = new Program (NULL);
  progTmp->setName("SpectroCrunch");
  queueTmp->addProgram(progTmp);
  progTmp = new Program (*progTmp);
  progTmp->setName("FastFocker");
  queueTmp->addProgram(progTmp);
  progTmp = new Program (*progTmp);
  progTmp->setName("SpeedSlater");
  queueTmp->addProgram(progTmp);
  m_connection = new TestConnection(this);

  qRegisterMetaType<MoleQueue::Connection*>("MoleQueue::Connection*");
  qRegisterMetaType<MoleQueue::EndpointId>("MoleQueue::EndpointId");
}

void JsonRpcTest::cleanupTestCase()
{
  delete m_connection;
}

void JsonRpcTest::init()
{
  // Reset error monitor
  m_error = false;
}

void JsonRpcTest::cleanup()
{

}

void JsonRpcTest::validateRequest()
{
  // Test valid requests
  m_packet = readReferenceString("jsonrpc-ref/valid-requests.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading valid requests file (not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (!m_rpc.validateRequest(*it, false)) {
        qDebug() << "Valid request failed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test invalid requests
  m_packet = readReferenceString("jsonrpc-ref/invalid-requests.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading invalid requests file (not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateRequest(*it, false)) {
        qDebug() << "Invalid request passed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test strictly invalid requests
  m_packet = readReferenceString("jsonrpc-ref/strictly-invalid-requests.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading strictly invalid requests file "
                "(not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateRequest(*it, true)) {
        qDebug() << "Strictly invalid request passed strict validation:";
        printNode(*it);
        m_error = true;
      }
      if (!m_rpc.validateRequest(*it, false)) {
        qDebug() << "Strictly invalid request failed loose validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::validateResponse()
{
  // Test valid responses
  m_packet = readReferenceString("jsonrpc-ref/valid-responses.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading valid responses file (not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (!m_rpc.validateResponse(*it, false)) {
        qDebug() << "Valid response failed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test invalid responses
  m_packet = readReferenceString("jsonrpc-ref/invalid-responses.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading invalid responses file (not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateResponse(*it, false)) {
        qDebug() << "Invalid response passed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test strictly invalid responses
  m_packet = readReferenceString("jsonrpc-ref/strictly-invalid-responses.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading strictly invalid responses file "
                "(not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateResponse(*it, true)) {
        qDebug() << "Strictly invalid response passed strict validation:";
        printNode(*it);
        m_error = true;
      }
      if (!m_rpc.validateResponse(*it, false)) {
        qDebug() << "Strictly invalid response failed loose validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::validateNotification()
{
  // Test valid notifications
  m_packet = readReferenceString("jsonrpc-ref/valid-notifications.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading valid notifications file (not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (!m_rpc.validateNotification(*it, false)) {
        qDebug() << "Valid notification failed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test invalid notifications
  m_packet = readReferenceString("jsonrpc-ref/invalid-notifications.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading invalid notifications file "
                "(not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateNotification(*it, false)) {
        qDebug() << "Invalid notification passed validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  // Test strictly invalid notifications
  m_packet = readReferenceString("jsonrpc-ref/strictly-invalid-notifications.json");
  m_reader.parse(m_packet.constData(), m_packet.constData() + m_packet.size(),
               m_root, false);
  if (!m_root.isArray()) {
    qDebug() << "Error reading strictly invalid notifications file "
                "(not a test failure).";
    m_error = true;
  }
  else {
    for (Json::Value::iterator it = m_root.begin(), it_end = m_root.end();
         it != it_end; ++it) {
      if (m_rpc.validateNotification(*it, true)) {
        qDebug() << "Strictly invalid notification passed strict validation:";
        printNode(*it);
        m_error = true;
      }
      if (!m_rpc.validateNotification(*it, false)) {
        qDebug() << "Strictly invalid notification failed loose validation:";
        printNode(*it);
        m_error = true;
      }
    }
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::generateJobRequest()
{
  Job req;
  req.setQueue("Some big ol' cluster");
  req.setProgram("Quantum Tater");
  req.setDescription("spud slicer 28");
  req.setInputAsPath("/tmp/myjob/test.potato");
  req.setInputAsString("This string will get ignored!");

  m_packet = m_rpc.generateJobRequest(&req, 14);
  if (!m_rpc.validateRequest(m_packet, true)) {
    qDebug() << "Job request packet failed validation!";
    m_error = true;
  }
  m_refPacket = readReferenceString("jsonrpc-ref/job-request.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Job request generation failed!";
    qDebug() << "Expected:" << m_refPacket;
    qDebug() << "Actual:" << m_packet;
    m_error = true;
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::generateJobSubmissionConfirmation()
{
  m_packet = m_rpc.generateJobSubmissionConfirmation(12, 789123,
                                                 "/tmp/myjob/test.potato", 14);
  if (!m_rpc.validateResponse(m_packet, true)) {
    qDebug() << "Job request response packet failed validation!";
    m_error = true;
  }
  m_refPacket = readReferenceString("jsonrpc-ref/job-submit-success.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Job request confirmation generation failed!";
    qDebug() << "Expected:" << m_refPacket;
    qDebug() << "Actual:" << m_packet;
    m_error = true;
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::generateErrorResponse()
{
  m_packet = m_rpc.generateErrorResponse(-32601,
                                     "Method not found: 'justDoWhatIWant'", 19);
  if (!m_rpc.validateResponse(m_packet, true)) {
    qDebug() << "Job request error response packet failed validation!";
    m_error = true;
  }
  m_refPacket = readReferenceString("jsonrpc-ref/error-response.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Job request error generation failed!";
    qDebug() << "Expected:" << m_refPacket;
    qDebug() << "Actual:" << m_packet;
    m_error = true;
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::generateJobCancellation()
{
  JobManager jobManager;
  Job req = jobManager.newJob();
  req.setMoleQueueId(0);
  req.setQueue("Some big ol' cluster");
  req.setProgram("Quantum Tater");
  req.setDescription("spud slicer 28");
  req.setInputAsPath("/tmp/myjob/test.potato");
  req.setInputAsString("This string will get ignored!");

  m_packet = m_rpc.generateJobCancellation(req, 15);
  if (!m_rpc.validateRequest(m_packet, true)) {
    qDebug() << "Job cancellation request packet failed validation!";
    m_error = true;
  }
  m_refPacket = readReferenceString("jsonrpc-ref/job-cancellation.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Job cancellation request generation failed!";
    qDebug() << "Expected:" << m_refPacket;
    qDebug() << "Actual:" << m_packet;
    m_error = true;
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::generateJobCancellationConfirmation()
{
  m_packet = m_rpc.generateJobCancellationConfirmation(18, 15);
  if (!m_rpc.validateResponse(m_packet, true)) {
    qDebug() << "Job cancellation response packet failed validation!";
    m_error = true;
  }
  m_refPacket = readReferenceString("jsonrpc-ref/job-cancellation-confirm.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Job cancellation confirmation generation failed!";
    qDebug() << "Expected:" << m_refPacket;
    qDebug() << "Actual:" << m_packet;
    m_error = true;
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::generateQueueListRequest()
{
  m_packet = m_rpc.generateQueueListRequest(23);
  if (!m_rpc.validateRequest(m_packet, true)) {
    qDebug() << "Queue list request packet failed validation!";
    m_error = true;
  }
  m_refPacket = readReferenceString("jsonrpc-ref/queue-list-request.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Queue list request failed!";
    qDebug() << "Expected:" << m_refPacket;
    qDebug() << "Actual:" << m_packet;
    m_error = true;
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::generateQueueList()
{
  m_packet = m_rpc.generateQueueList(m_qmanager.toQueueList(), 23);
  if (!m_rpc.validateResponse(m_packet, true)) {
    qDebug() << "Queue list response packet failed validation!";
    m_error = true;
  }
  m_refPacket = readReferenceString("jsonrpc-ref/queue-list.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Queue list generation failed!";
    qDebug() << "Expected:" << m_refPacket;
    qDebug() << "Actual:" << m_packet;
    m_error = true;
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::generateJobStateChangeNotification()
{
  m_packet = m_rpc.generateJobStateChangeNotification(12, RunningRemote,
                                                      Finished);
  if (!m_rpc.validateNotification(m_packet, true)) {
    qDebug() << "Job state change notification packet failed validation!";
    m_error = true;
  }
  m_refPacket = readReferenceString("jsonrpc-ref/jobstate-change.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Job state change notification generation failed!";
    qDebug() << "Expected:" << m_refPacket;
    qDebug() << "Actual:" << m_packet;
    m_error = true;
  }

  QVERIFY(m_error == false);
}

void JsonRpcTest::interpretIncomingPacket_unparsable()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    invalidPacketReceived(MoleQueue::Connection*,MoleQueue::EndpointId,
                                          Json::Value,Json::Value)));
  m_packet = "[I'm not valid JSON!}";
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
  QVariantList list = spy.takeFirst();
  QCOMPARE(list.size(), 4);
  ///TODO verify that the connection is provided ...
  QVERIFY(list[2].value<Json::Value>().isNull());
  QCOMPARE(list[3].value<Json::Value>()["receivedPacket"].asCString(),
           m_packet.constData());
}

void JsonRpcTest::interpretIncomingPacket_invalidRequest()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    invalidRequestReceived(MoleQueue::Connection*,MoleQueue::EndpointId,
                                           Json::Value,Json::Value)));
  m_packet = "{\"I'm valid JSON\" : \"but not JSON-RPC\"}";
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingPacket_unrecognizedRequest()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    unrecognizedRequestReceived(MoleQueue::Connection*,MoleQueue::EndpointId,
                                                Json::Value,Json::Value)));
  m_packet = "{ \"jsonrpc\" : \"2.0\", \"id\" : 0, \"method\" : \"notReal\" }";
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingPacket_listQueuesRequest()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    queueListRequestReceived(MoleQueue::Connection*,MoleQueue::EndpointId,
                                             MoleQueue::IdType)));
  m_packet = readReferenceString("jsonrpc-ref/queue-list-request.json");
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingPacket_listQueuesResult()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    queueListReceived(MoleQueue::IdType,
                                      MoleQueue::QueueListType)));
  // Register the packet id with this method for JsonRpc:
  m_rpc.generateQueueListRequest(23);
  m_packet = readReferenceString("jsonrpc-ref/queue-list.json");
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingPacket_listQueuesError()
{
  // Nothing to do, can't happen.
}

void JsonRpcTest::interpretIncomingPacket_submitJobRequest()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    jobSubmissionRequestReceived(MoleQueue::Connection*,
                                                 MoleQueue::EndpointId,
                                                 MoleQueue::IdType,QVariantHash)));
  m_packet = readReferenceString("jsonrpc-ref/job-request.json");
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingPacket_submitJobResult()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    successfulSubmissionReceived(MoleQueue::IdType,
                                                 MoleQueue::IdType,
                                                 MoleQueue::IdType,QDir)));
  // Register the packet id with this method for JsonRpc:
  m_rpc.generateJobSubmissionConfirmation(0,0,"",14);
  m_packet = readReferenceString("jsonrpc-ref/job-submit-success.json");
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingPacket_submitJobError()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    failedSubmissionReceived(MoleQueue::IdType,
                                             MoleQueue::JobSubmissionErrorCode,
                                             QString)));
  // Create the error response first
  m_packet = m_rpc.generateErrorResponse(None, "Not a real error!",
                                         Json::nullValue, 15);
  // Register the packet id with this method for JsonRpc:
  MoleQueue::Job req;
  m_rpc.generateJobRequest(&req, 15);
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingPacket_cancelJobRequest()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    jobCancellationRequestReceived(MoleQueue::Connection*,
                                                   MoleQueue::EndpointId,
                                                   MoleQueue::IdType,
                                                   MoleQueue::IdType)));
  m_packet = readReferenceString("jsonrpc-ref/job-cancellation.json");
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingPacket_cancelJobResult()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    jobCancellationConfirmationReceived(MoleQueue::IdType,
                                                        MoleQueue::IdType)));
  // Register the packet id with this method for JsonRpc:
  MoleQueue::Job req;
  m_rpc.generateJobCancellation(&req, 15);
  m_packet = readReferenceString("jsonrpc-ref/job-cancellation-confirm.json");
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
}

void JsonRpcTest::interpretIncomingPacket_cancelJobError()
{
  // Not implemented
}

void JsonRpcTest::interpretIncomingPacket_jobStateChange()
{
  QSignalSpy spy (&m_rpc, SIGNAL(
                    jobStateChangeReceived(MoleQueue::IdType,MoleQueue::JobState,
                                           MoleQueue::JobState)));
  m_packet = readReferenceString("jsonrpc-ref/jobstate-change.json");
  m_rpc.interpretIncomingPacket(m_connection, m_packet);

  QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(JsonRpcTest)

#include "moc_jsonrpctest.cxx"
