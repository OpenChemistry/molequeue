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

#include "serverjsonrpc.h"

#include "filespecification.h"
#include "job.h"
#include "jobdata.h"
#include "jobmanager.h"
#include "program.h"
#include "queue.h"
#include "queuemanager.h"
#include "queues/local.h"
#include "queues/sge.h"
#include "transport/connection.h"
#include "transport/message.h"

#include <json/json.h>

#include <QtTest>
#include <QtCore/QFile>

using namespace MoleQueue;

class ServerJsonRpcTest : public QObject
{
  Q_OBJECT

private:
  PacketType readReferenceString(const QString &filename);
  void printNode(const Json::Value &);

  bool m_error;
  ServerJsonRpc m_rpc;
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

  void generateJobSubmissionConfirmation();
  void generateJobCancellationConfirmation();
  void generateLookupJobResponse();
  void generateQueueList();
  void generateJobStateChangeNotification();

  void interpretIncomingMessage_submitJobRequest();
  void interpretIncomingMessage_cancelJobRequest();
};

class TestConnection : public Connection
{
  Q_OBJECT
public:
  TestConnection(QObject *parentObject = 0 ) : Connection(parentObject) {};

  void open() {};
  void start() {};
  void send(const Message &msg) { Q_UNUSED(msg); };
  void close() {};
  bool isOpen() { return false; };
  QString connectionString() const { return ""; };
};


PacketType ServerJsonRpcTest::readReferenceString(const QString &filename)
{
  QString realFilename = MoleQueue_TESTDATA_DIR + filename;
  QFile refFile (realFilename);
  if (!refFile.open(QFile::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot access reference file" << realFilename;
    return "";
  }
  PacketType contents = refFile.readAll();
  refFile.close();
  return contents;
}

void ServerJsonRpcTest::printNode(const Json::Value &root)
{
  std::string str;
  str = m_writer.write(root);
  qDebug() << str.c_str();
}

void ServerJsonRpcTest::initTestCase()
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
  qRegisterMetaType<MoleQueue::EndpointIdType>("MoleQueue::EndpointIdType");
}

void ServerJsonRpcTest::cleanupTestCase()
{

}

void ServerJsonRpcTest::init()
{
  // Reset error monitor
  m_error = false;
}

void ServerJsonRpcTest::cleanup()
{
}

void ServerJsonRpcTest::generateJobSubmissionConfirmation()
{
  m_packet = m_rpc.generateJobSubmissionConfirmation(12,
                                                     "/tmp/myjob/test.potato",
                                                     "14");
  if (!m_rpc.validateResponse(Message(m_packet), true)) {
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

void ServerJsonRpcTest::generateJobCancellationConfirmation()
{
  m_packet = m_rpc.generateJobCancellationConfirmation(18, "15");
  if (!m_rpc.validateResponse(Message(m_packet), true)) {
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

void ServerJsonRpcTest::generateLookupJobResponse()
{
  JobManager jobManager;
  Job req = jobManager.newJob();
  req.setMoleQueueId(17);
  req.setQueueId(7366);
  req.setQueue("Some big ol' cluster");
  req.setProgram("Quantum Tater");
  req.setDescription("spud slicer 28");
  req.setInputFile(FileSpecification(QString("/tmp/myjob/test.potato")));

  // Test successful lookup
  m_packet = m_rpc.generateLookupJobResponse(req, req.moleQueueId(), "12");
  if (!m_rpc.validateResponse(Message(m_packet), true)) {
    qDebug() << "Successful job lookup response packet failed validation!";
    m_error = true;
  }
  m_refPacket = readReferenceString("jsonrpc-ref/lookupJob-response.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Successful job lookup response generation failed!" << endl
             << "Expected:" << m_refPacket << endl
             << "Actual:" << m_packet;
    m_error = true;
  }

  // Test unsuccessful lookup
  m_packet = m_rpc.generateLookupJobResponse(Job(), 32, "12");
  if (!m_rpc.validateResponse(Message(m_packet), true)) {
    qDebug() << "Unsuccessful job lookup response packet failed validation!";
    m_error = true;
  }
  m_refPacket = readReferenceString("jsonrpc-ref/lookupJob-error.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Unsuccessful job lookup response generation failed!" << endl
             << "Expected:" << m_refPacket << endl
             << "Actual:" << m_packet;
    m_error = true;
  }

  QVERIFY(m_error == false);
}

void ServerJsonRpcTest::generateQueueList()
{
  m_packet = m_rpc.generateQueueList(m_qmanager.toQueueList(), "23");
  if (!m_rpc.validateResponse(Message(m_packet), true)) {
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

void ServerJsonRpcTest::generateJobStateChangeNotification()
{
  m_packet = m_rpc.generateJobStateChangeNotification(12, RunningRemote,
                                                      Finished);
  if (!m_rpc.validateNotification(Message(m_packet), true)) {
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

void ServerJsonRpcTest::interpretIncomingMessage_submitJobRequest()
{
  QSignalSpy spy(&m_rpc, SIGNAL(jobSubmissionRequestReceived(MoleQueue::Message,
                                                             QVariantHash)));

  m_packet = readReferenceString("jsonrpc-ref/job-request.json");
  m_rpc.interpretIncomingMessage(Message(m_connection, EndpointIdType(),
                                         m_packet));

  QCOMPARE(spy.count(), 1);
}

void ServerJsonRpcTest::interpretIncomingMessage_cancelJobRequest()
{
  QSignalSpy spy(&m_rpc,
                 SIGNAL(jobCancellationRequestReceived(MoleQueue::Message,
                                                       MoleQueue::IdType)));

  m_packet = readReferenceString("jsonrpc-ref/job-cancellation.json");
  m_rpc.interpretIncomingMessage(Message(m_connection, EndpointIdType(),
                                         m_packet));

  QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(ServerJsonRpcTest)

#include "serverjsonrpctest.moc"
