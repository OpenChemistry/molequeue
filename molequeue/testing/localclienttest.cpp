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

#include <molequeue/client/client.h>
#include <molequeue/client/job.h>

#include <unistd.h>

#include "testserver.h"

#include <QtGui/QApplication>

using MoleQueue::PacketType;

class LocalClientTest : public QObject
{
  Q_OBJECT

private:
  TestServer *m_server;
  MoleQueue::Client m_client;
  PacketType m_packet;

private slots:
  PacketType readReferenceString(const QString &filename);

  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void isConnected();

  void listQueues();

  void jobSubmission();

  void jobLookup();
};

PacketType LocalClientTest::readReferenceString(const QString &filename)
{
  QString realFilename = TESTDATADIR + filename;
  QFile refFile(realFilename);
  if (!refFile.open(QFile::ReadOnly | QFile::Text)) {
    qDebug() << "Cannot access reference file" << realFilename;
    return "";
  }
  PacketType contents = refFile.readAll();
  refFile.close();
  return contents;
}

void LocalClientTest::initTestCase()
{
  m_server = new TestServer(&m_packet);
  qDebug() << m_server->socketName();
  m_client.connectToServer(/*m_server->socketName()*/);
  // Let the event loop run a bit to handle the connections
  qApp->processEvents(QEventLoop::AllEvents, 1000);
}

void LocalClientTest::cleanupTestCase()
{
//  delete m_server;
  //delete m_client;
}

void LocalClientTest::init()
{
//  m_packet.clear();
}

void LocalClientTest::cleanup()
{
}

void LocalClientTest::isConnected()
{
  QVERIFY(m_client.isConnected());
}

void LocalClientTest::listQueues()
{
  m_client.requestQueueList();
  qApp->processEvents(QEventLoop::AllEvents, 2000);
}

void LocalClientTest::jobSubmission()
{
  MoleQueue::JobObject request;

  request.setValue("queue", "Local");
  request.setValue("program", "sleep");
  request.setValue("description", "Test job");
  m_client.submitJob(request);
  qApp->processEvents(QEventLoop::AllEvents, 2000);

  //QVERIFY2(m_server->waitForPacket(), "Timeout waiting for reply.");

  PacketType refPacket =
      readReferenceString("client-ref/job-submission.json");

  // Strip out the random ids in the packets
  QRegExp strip("\\n\\s+\"id\"\\s+:\\s+\\d+\\s*,\\s*\\n");
  QString strippedPacket = QString(m_packet);
  QString strippedRefPacket = QString(refPacket);

  strippedPacket.replace(strip, "\n");
  strippedRefPacket.replace(strip, "\n");

  QCOMPARE(strippedPacket, strippedRefPacket);
}

void LocalClientTest::jobLookup()
{
  m_client.lookupJob(2);
  qApp->processEvents(QEventLoop::AllEvents, 2000);

  sleep(1);

  qApp->processEvents(QEventLoop::AllEvents, 2000);
}

QTEST_MAIN(LocalClientTest)

#include "localclienttest.moc"
