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
#include <QtCore/QFile>

#include "jsonrpc.h"

#include "jobrequest.h"
#include "program.h"
#include "queue.h"
#include "queuemanager.h"

#include "thirdparty/jsoncpp/json/json.h"

using namespace MoleQueue;

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
  m_rpc.setDebug(true);

  Queue *queueTmp = m_qmanager.createQueue("Remote - SGE");
  m_qmanager.addQueue(queueTmp);
  queueTmp->setName("Some big ol' cluster");
  Program *progTmp = new Program (NULL);
  progTmp->setName("Quantum Tater");
  queueTmp->addProgram(progTmp);
  progTmp = new Program (*progTmp);
  progTmp->setName("Crystal Math");
  queueTmp->addProgram(progTmp);
  progTmp = new Program (*progTmp);
  progTmp->setName("Nebulous Nucleus");
  queueTmp->addProgram(progTmp);
  queueTmp = m_qmanager.createQueue("Local");
  m_qmanager.addQueue(queueTmp);
  queueTmp->setName("Puny local queue");
  progTmp = new Program (NULL);
  progTmp->setName("SpectroCrunch");
  queueTmp->addProgram(progTmp);
  progTmp = new Program (*progTmp);
  progTmp->setName("FastFocker");
  queueTmp->addProgram(progTmp);
  progTmp = new Program (*progTmp);
  progTmp->setName("SpeedSlater");
  queueTmp->addProgram(progTmp);
}

void JsonRpcTest::cleanupTestCase()
{
  const QList<Queue*> queues = m_qmanager.queues();

  for (QList<Queue*>::const_iterator it = queues.constBegin(),
       it_end = queues.constEnd(); it != it_end; ++it) {
    foreach (const QString &prog, (*it)->programs()) {
      delete (*it)->program(prog);
    }
  }

  qDeleteAll(m_qmanager.queues());
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
  JobRequest req (NULL);
  req.setQueue("Some big ol' cluster");
  req.setProgram("Quantum Tater");
  req.setDescription("spud slicer 28");
  req.setInputAsPath("/tmp/myjob/test.potato");
  req.setInputAsString("This string will get ignored!");

  m_packet = m_rpc.generateJobRequest(req, 14);
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
  JobRequest req (NULL);
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
  m_packet = m_rpc.generateQueueList(&m_qmanager, 23);
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

QTEST_MAIN(JsonRpcTest)

#include "moc_jsonrpctest.cxx"
