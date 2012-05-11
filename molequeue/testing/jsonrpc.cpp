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

#include <json/json.h>

using namespace MoleQueue;

mqPacketType readReferenceString(const QString &filename);
void printNode(const Json::Value &);

int jsonrpc(int /*argc*/, char */*argv*/[])
{
  bool error = false;

  // Create testing objects
  JsonRpc rpc;
//  rpc.setDebug(true);
  mqPacketType packet;
  mqPacketType refPacket;

  JobRequest req (NULL);
  req.setQueue("Some big ol' cluster");
  req.setProgram("Quantum Tater");
  req.setDescription("spud slicer 28");
  req.setInputAsPath("/tmp/myjob/test.potato");
  req.setInputAsString("This string will get ignored!");

  QueueManager qmanager (NULL);
  Queue *queueTmp = qmanager.createQueue("Remote - SGE");
  qmanager.addQueue(queueTmp);
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
  queueTmp = qmanager.createQueue("Local");
  qmanager.addQueue(queueTmp);
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

  Json::Reader reader;
  Json::Value root;

  /////////////////////////////////
  // Test the validation methods //
  /////////////////////////////////

  // Test valid requests
  packet = readReferenceString("jsonrpc-ref/valid-requests.json");
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  if (!root.isArray()) {
    qDebug() << "Error reading valid requests file (not a test failure).";
    error = true;
  }
  else {
    for (Json::Value::iterator it = root.begin(), it_end = root.end();
         it != it_end; ++it) {
      if (!rpc.validateRequest(*it, false)) {
        qDebug() << "Valid request failed validation:";
        printNode(*it);
        error = true;
      }
    }
  }

  // Test invalid requests
  packet = readReferenceString("jsonrpc-ref/invalid-requests.json");
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  if (!root.isArray()) {
    qDebug() << "Error reading invalid requests file (not a test failure).";
    error = true;
  }
  else {
    for (Json::Value::iterator it = root.begin(), it_end = root.end();
         it != it_end; ++it) {
      if (rpc.validateRequest(*it, false)) {
        qDebug() << "Invalid request passed validation:";
        printNode(*it);
        error = true;
      }
    }
  }

  // Test strictly invalid requests
  packet = readReferenceString("jsonrpc-ref/strictly-invalid-requests.json");
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  if (!root.isArray()) {
    qDebug() << "Error reading strictly invalid requests file "
                "(not a test failure).";
    error = true;
  }
  else {
    for (Json::Value::iterator it = root.begin(), it_end = root.end();
         it != it_end; ++it) {
      if (rpc.validateRequest(*it, true)) {
        qDebug() << "Strictly invalid request passed strict validation:";
        printNode(*it);
        error = true;
      }
      if (!rpc.validateRequest(*it, false)) {
        qDebug() << "Strictly invalid request failed loose validation:";
        printNode(*it);
        error = true;
      }
    }
  }

  // Test valid responses
  packet = readReferenceString("jsonrpc-ref/valid-responses.json");
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  if (!root.isArray()) {
    qDebug() << "Error reading valid responses file (not a test failure).";
    error = true;
  }
  else {
    for (Json::Value::iterator it = root.begin(), it_end = root.end();
         it != it_end; ++it) {
      if (!rpc.validateResponse(*it, false)) {
        qDebug() << "Valid response failed validation:";
        printNode(*it);
        error = true;
      }
    }
  }

  // Test invalid responses
  packet = readReferenceString("jsonrpc-ref/invalid-responses.json");
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  if (!root.isArray()) {
    qDebug() << "Error reading invalid responses file (not a test failure).";
    error = true;
  }
  else {
    for (Json::Value::iterator it = root.begin(), it_end = root.end();
         it != it_end; ++it) {
      if (rpc.validateResponse(*it, false)) {
        qDebug() << "Invalid response passed validation:";
        printNode(*it);
        error = true;
      }
    }
  }

  // Test strictly invalid responses
  packet = readReferenceString("jsonrpc-ref/strictly-invalid-responses.json");
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  if (!root.isArray()) {
    qDebug() << "Error reading strictly invalid responses file "
                "(not a test failure).";
    error = true;
  }
  else {
    for (Json::Value::iterator it = root.begin(), it_end = root.end();
         it != it_end; ++it) {
      if (rpc.validateResponse(*it, true)) {
        qDebug() << "Strictly invalid response passed strict validation:";
        printNode(*it);
        error = true;
      }
      if (!rpc.validateResponse(*it, false)) {
        qDebug() << "Strictly invalid response failed loose validation:";
        printNode(*it);
        error = true;
      }
    }
  }

  // Test valid notifications
  packet = readReferenceString("jsonrpc-ref/valid-notifications.json");
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  if (!root.isArray()) {
    qDebug() << "Error reading valid notifications file (not a test failure).";
    error = true;
  }
  else {
    for (Json::Value::iterator it = root.begin(), it_end = root.end();
         it != it_end; ++it) {
      if (!rpc.validateNotification(*it, false)) {
        qDebug() << "Valid notification failed validation:";
        printNode(*it);
        error = true;
      }
    }
  }

  // Test invalid notifications
  packet = readReferenceString("jsonrpc-ref/invalid-notifications.json");
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  if (!root.isArray()) {
    qDebug() << "Error reading invalid notifications file "
                "(not a test failure).";
    error = true;
  }
  else {
    for (Json::Value::iterator it = root.begin(), it_end = root.end();
         it != it_end; ++it) {
      if (rpc.validateNotification(*it, false)) {
        qDebug() << "Invalid notification passed validation:";
        printNode(*it);
        error = true;
      }
    }
  }

  // Test strictly invalid notifications
  packet = readReferenceString("jsonrpc-ref/strictly-invalid-notifications.json");
  reader.parse(packet.constData(), packet.constData() + packet.size(),
               root, false);
  if (!root.isArray()) {
    qDebug() << "Error reading strictly invalid notifications file "
                "(not a test failure).";
    error = true;
  }
  else {
    for (Json::Value::iterator it = root.begin(), it_end = root.end();
         it != it_end; ++it) {
      if (rpc.validateNotification(*it, true)) {
        qDebug() << "Strictly invalid notification passed strict validation:";
        printNode(*it);
        error = true;
      }
      if (!rpc.validateNotification(*it, false)) {
        qDebug() << "Strictly invalid notification failed loose validation:";
        printNode(*it);
        error = true;
      }
    }
  }

  /////////////////////////////////
  // Test json packet generators //
  /////////////////////////////////

  // Job request generation:
  packet = rpc.generateJobRequest(req, 14);
  if (!rpc.validateRequest(packet, true)) {
    qDebug() << "Job request packet failed validation!";
    error = true;
  }
  refPacket = readReferenceString("jsonrpc-ref/job-request.json");
  if (packet != refPacket) {
    qDebug() << "Job request generation failed!";
    qDebug() << "Expected:" << refPacket;
    qDebug() << "Actual:" << packet;
    error = true;
  }

  // Job request confirmation:
  packet = rpc.generateJobSubmissionConfirmation(12, 789123,
                                                 "/tmp/myjob/test.potato", 14);
  if (!rpc.validateResponse(packet, true)) {
    qDebug() << "Job request response packet failed validation!";
    error = true;
  }
  refPacket = readReferenceString("jsonrpc-ref/job-submit-success.json");
  if (packet != refPacket) {
    qDebug() << "Job request confirmation generation failed!";
    qDebug() << "Expected:" << refPacket;
    qDebug() << "Actual:" << packet;
    error = true;
  }

  // Error response. Ensure that the errorCode type can handle negative ints:
  packet = rpc.generateErrorResponse(-32601,
                                     "Method not found: 'justDoWhatIWant'", 19);
  if (!rpc.validateResponse(packet, true)) {
    qDebug() << "Job request error response packet failed validation!";
    error = true;
  }
  refPacket = readReferenceString("jsonrpc-ref/error-response.json");
  if (packet != refPacket) {
    qDebug() << "Job request error generation failed!";
    qDebug() << "Expected:" << refPacket;
    qDebug() << "Actual:" << packet;
    error = true;
  }

  // Job cancellation:
  packet = rpc.generateJobCancellation(req, 15);
  if (!rpc.validateRequest(packet, true)) {
    qDebug() << "Job cancellation request packet failed validation!";
    error = true;
  }
  refPacket = readReferenceString("jsonrpc-ref/job-cancellation.json");
  if (packet != refPacket) {
    qDebug() << "Job cancellation request generation failed!";
    qDebug() << "Expected:" << refPacket;
    qDebug() << "Actual:" << packet;
    error = true;
  }

  // Job cancellation confirmation:
  packet = rpc.generateJobCancellationConfirmation(18, 15);
  if (!rpc.validateResponse(packet, true)) {
    qDebug() << "Job cancellation response packet failed validation!";
    error = true;
  }
  refPacket = readReferenceString("jsonrpc-ref/job-cancellation-confirm.json");
  if (packet != refPacket) {
    qDebug() << "Job cancellation confirmation generation failed!";
    qDebug() << "Expected:" << refPacket;
    qDebug() << "Actual:" << packet;
    error = true;
  }

  // Queue list request:
  packet = rpc.generateQueueListRequest(23);
  if (!rpc.validateRequest(packet, true)) {
    qDebug() << "Queue list request packet failed validation!";
    error = true;
  }
  refPacket = readReferenceString("jsonrpc-ref/queue-list-request.json");
  if (packet != refPacket) {
    qDebug() << "Queue list request failed!";
    qDebug() << "Expected:" << refPacket;
    qDebug() << "Actual:" << packet;
    error = true;
  }

  // Queue list generation
  packet = rpc.generateQueueList(&qmanager, 23);
  if (!rpc.validateResponse(packet, true)) {
    qDebug() << "Queue list response packet failed validation!";
    error = true;
  }
  refPacket = readReferenceString("jsonrpc-ref/queue-list.json");
  if (packet != refPacket) {
    qDebug() << "Queue list generation failed!";
    qDebug() << "Expected:" << refPacket;
    qDebug() << "Actual:" << packet;
    error = true;
  }

  // State change notification
  packet = rpc.generateJobStateChangeNotification(12, RunningRemote, Finished);
  if (!rpc.validateNotification(packet, true)) {
    qDebug() << "Job state change notification packet failed validation!";
    error = true;
  }
  refPacket = readReferenceString("jsonrpc-ref/jobstate-change.json");
  if (packet != refPacket) {
    qDebug() << "Job state change notification generation failed!";
    qDebug() << "Expected:" << refPacket;
    qDebug() << "Actual:" << packet;
    error = true;
  }

  return error ? 1 : 0;
}

mqPacketType readReferenceString(const QString &filename)
{
  QString realFilename = TESTDATADIR + filename;
  QFile refFile (realFilename);
  if (!refFile.open(QFile::ReadOnly)) {
    qDebug() << "Cannot access reference file" << realFilename;
    return "";
  }
  mqPacketType contents = refFile.readAll();
  refFile.close();
  return contents;
}

void printNode(const Json::Value &root)
{
  std::string str;
  Json::StyledWriter writer;
  str = writer.write(root);
  qDebug() << str.c_str();
}
