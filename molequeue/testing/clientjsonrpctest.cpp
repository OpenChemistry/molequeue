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

#include "clientjsonrpc.h"

#include "filespecification.h"
#include "job.h"
#include "jobdata.h"
#include "jobmanager.h"
#include "molequeuetestconfig.h"
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

class ClientJsonRpcTest : public QObject
{
  Q_OBJECT

private:
  PacketType readReferenceString(const QString &filename);
  void printNode(const Json::Value &);

  bool m_error;
  ClientJsonRpc m_rpc;
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
  void generateJobRequest();
  void generateJobCancellation();
  void generateLookupJobRequest();
  void generateQueueListRequest();

  void interpretIncomingPacket_listQueuesResult();
  void interpretIncomingPacket_listQueuesError();
  void interpretIncomingPacket_submitJobResult();
  void interpretIncomingPacket_submitJobError();
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
  void send(const Message &msg) { Q_UNUSED(msg); };
  void close() {};
  bool isOpen() { return false; };
  QString connectionString() const { return ""; };
};


PacketType ClientJsonRpcTest::readReferenceString(const QString &filename)
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

void ClientJsonRpcTest::printNode(const Json::Value &root)
{
  std::string str;
  str = m_writer.write(root);
  qDebug() << str.c_str();
}

void ClientJsonRpcTest::initTestCase()
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
  qRegisterMetaType<MoleQueue::EndpointIdType>("MoleQueue::EndpointId");
}

void ClientJsonRpcTest::cleanupTestCase()
{

}

void ClientJsonRpcTest::init()
{
  // Reset error monitor
  m_error = false;
}

void ClientJsonRpcTest::cleanup()
{

}

void ClientJsonRpcTest::generateJobRequest()
{
  JobManager jobManager;
  Job req = jobManager.newJob();
  req.setMoleQueueId(0);
  req.setQueue("Some big ol' cluster");
  req.setProgram("Quantum Tater");
  req.setDescription("spud slicer 28");
  req.setInputFile(FileSpecification(QString("/tmp/myjob/test.potato")));

  m_packet = m_rpc.generateJobRequest(req, "14");
  if (!m_rpc.validateRequest(Message(m_packet), true)) {
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

void ClientJsonRpcTest::generateJobCancellation()
{
  JobManager jobManager;
  Job req = jobManager.newJob();
  req.setMoleQueueId(0);
  req.setQueue("Some big ol' cluster");
  req.setProgram("Quantum Tater");
  req.setDescription("spud slicer 28");
  req.setInputFile(FileSpecification(QString("/tmp/myjob/test.potato")));

  m_packet = m_rpc.generateJobCancellation(req, "15");
  if (!m_rpc.validateRequest(Message(m_packet), true)) {
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

void ClientJsonRpcTest::generateLookupJobRequest()
{
  m_packet = m_rpc.generateLookupJobRequest(17, "12");
  if (!m_rpc.validateRequest(Message(m_packet), true)) {
    qDebug() << "Job lookup request packet failed validation!";
    m_error = true;
  }

  m_refPacket = readReferenceString("jsonrpc-ref/lookupJob-request.json");
  if (m_packet != m_refPacket) {
    qDebug() << "Job lookup request generation failed!" << endl
             << "Expected:" << m_refPacket << endl
             << "Actual:" << m_packet;
    m_error = true;
  }

  QVERIFY(m_error == false);
}

void ClientJsonRpcTest::generateQueueListRequest()
{
  m_packet = m_rpc.generateQueueListRequest("23");
  if (!m_rpc.validateRequest(Message(m_packet), true)) {
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

void ClientJsonRpcTest::interpretIncomingPacket_listQueuesResult()
{
  QSignalSpy spy(&m_rpc, SIGNAL(queueListReceived(MoleQueue::MessageIdType,
                                                  MoleQueue::QueueListType)));

  // Register the packet id with this method for JsonRpc:
  m_rpc.generateQueueListRequest("23");
  m_packet = readReferenceString("jsonrpc-ref/queue-list.json");
  m_rpc.interpretIncomingMessage(Message(m_connection, EndpointIdType(),
                                         m_packet));

  QCOMPARE(spy.count(), 1);
}

void ClientJsonRpcTest::interpretIncomingPacket_listQueuesError()
{
  // Nothing to do, can't happen.
}

void ClientJsonRpcTest::interpretIncomingPacket_submitJobResult()
{
  QSignalSpy spy(&m_rpc,
                 SIGNAL(successfulSubmissionReceived(MoleQueue::MessageIdType,
                                                     MoleQueue::IdType,QDir)));

  // Register the packet id with this method for JsonRpc:
  m_rpc.generateJobRequest(0,"14");
  m_packet = readReferenceString("jsonrpc-ref/job-submit-success.json");
  m_rpc.interpretIncomingMessage(Message(m_connection, EndpointIdType(),
                                         m_packet));

  QCOMPARE(spy.count(), 1);
}

void ClientJsonRpcTest::interpretIncomingPacket_submitJobError()
{
  QSignalSpy spy(&m_rpc,
                 SIGNAL(failedSubmissionReceived(MoleQueue::MessageIdType,
                                                 MoleQueue::ErrorCode,
                                                 QString)));

  // Create the error response first
  m_packet = m_rpc.generateErrorResponse(0, "Not a real error!",
                                         Json::nullValue, MessageIdType("15"));
  // Register the packet id with this method for JsonRpc:
  MoleQueue::Job req;
  m_rpc.generateJobRequest(req, "15");
  m_rpc.interpretIncomingMessage(Message(m_connection, EndpointIdType(),
                                         m_packet));

  QCOMPARE(spy.count(), 1);
}

void ClientJsonRpcTest::interpretIncomingPacket_cancelJobResult()
{
  QSignalSpy spy(&m_rpc, SIGNAL(jobCancellationConfirmationReceived(
                                  MoleQueue::MessageIdType,
                                  MoleQueue::IdType)));

  // Register the packet id with this method for JsonRpc:
  MoleQueue::Job req;
  m_rpc.generateJobCancellation(req, "15");
  m_packet = readReferenceString("jsonrpc-ref/job-cancellation-confirm.json");
  m_rpc.interpretIncomingMessage(Message(m_connection, EndpointIdType(),
                                         m_packet));

  QCOMPARE(spy.count(), 1);
}

void ClientJsonRpcTest::interpretIncomingPacket_cancelJobError()
{
  // Not implemented
}

void ClientJsonRpcTest::interpretIncomingPacket_jobStateChange()
{
  QSignalSpy spy(&m_rpc, SIGNAL(jobStateChangeReceived(
                                  MoleQueue::IdType, MoleQueue::JobState,
                                  MoleQueue::JobState)));

  m_packet = readReferenceString("jsonrpc-ref/jobstate-change.json");
  m_rpc.interpretIncomingMessage(Message(m_connection, EndpointIdType(),
                                         m_packet));

  QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(ClientJsonRpcTest)

#include "clientjsonrpctest.moc"
