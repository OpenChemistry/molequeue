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

#include "abstractrpcinterface.h"

#include "molequeueglobal.h"

#include <QtGui/QApplication>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

class TestServer : public QObject
{
  Q_OBJECT
  MoleQueue::PacketType *m_target;
  QLocalServer *m_server;
  QLocalSocket *m_socket;
public:
  TestServer(MoleQueue::PacketType *target)
    : QObject(NULL), m_target(target), m_server(new QLocalServer),
      m_socket(NULL)
  {
    if (!m_server->listen("MoleQueue")) {
      qWarning() << "Cannot start test server:" << m_server->errorString();
      return;
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
  }

  ~TestServer()
  {
    if (m_socket != NULL)
      m_socket->disconnect();

    delete m_server;
  }

  void sendPacket(const MoleQueue::PacketType &packet)
  {
    QDataStream out (m_socket);
    out.setVersion(QDataStream::Qt_4_7);
//    qDebug() << "Test server writing" << packet.size() << "bytes.";

    // Create header
    out << static_cast<quint32>(1);
    out << static_cast<quint32>(packet.size());
    // write data
    out << packet;
    m_socket->flush();
  }

private slots:
  void newConnection()
  {
    m_socket = m_server->nextPendingConnection();
    connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
//    qDebug() << "New connection!";
  }

  void readyRead()
  {
//  qDebug() << "Test server received" << m_socket->bytesAvailable() << "bytes.";
    QDataStream in (m_socket);
    in.setVersion(QDataStream::Qt_4_7);

    quint32 version;
    quint32 size;
    MoleQueue::PacketType packet;

    in >> version;
    in >> size;
    in >> packet;

//    qDebug() << "Received packet. Version" << version << "Size" << size;

    m_target->append(packet);
  }
};

class AbstractRpcInterfaceTest : public QObject
{
  Q_OBJECT

private:
  TestServer *m_server;
  MoleQueue::AbstractRpcInterface *m_rpc;
  MoleQueue::PacketType m_packet;

private slots:
  MoleQueue::PacketType readReferenceString(const QString &filename);

  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void testInvalidPacket();
  void testInvalidRequest();
  void testInvalidMethod();
  void testInvalidParams();
  void testInternalError();
};


MoleQueue::PacketType
AbstractRpcInterfaceTest::readReferenceString(const QString &filename)
{
  QString realFilename = TESTDATADIR + filename;
  QFile refFile (realFilename);
  if (!refFile.open(QFile::ReadOnly)) {
    qDebug() << "Cannot access reference file" << realFilename;
    return "";
  }
  MoleQueue::PacketType contents = refFile.readAll();
  refFile.close();
  return contents;
}

void AbstractRpcInterfaceTest::initTestCase()
{
  m_server = new TestServer(&m_packet);
  m_rpc = new MoleQueue::AbstractRpcInterface();
  // Let the event loop run a bit to handle the connections
  qApp->processEvents(QEventLoop::AllEvents, 1000);
}

void AbstractRpcInterfaceTest::cleanupTestCase()
{
  delete m_server;
  delete m_rpc;
}

void AbstractRpcInterfaceTest::init()
{
  m_packet.clear();
}

void AbstractRpcInterfaceTest::cleanup()
{
}

void AbstractRpcInterfaceTest::testInvalidPacket()
{
  MoleQueue::PacketType packet =
      "{ 42 \"I'm malformed JSON! ]";
  m_server->sendPacket(packet);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  MoleQueue::PacketType refPacket =
      this->readReferenceString("abstractrpcinterface-ref/invalid-json.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void AbstractRpcInterfaceTest::testInvalidRequest()
{
  MoleQueue::PacketType packet = "[1]"; // not an object
  m_server->sendPacket(packet);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  MoleQueue::PacketType refPacket =
      this->readReferenceString("abstractrpcinterface-ref/invalid-request.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void AbstractRpcInterfaceTest::testInvalidMethod()
{
  MoleQueue::PacketType packet =
      "{ \"jsonrpc\" : \"2.0\", \"id\" : 0, \"method\" : \"notARealMethod\"}";
  m_server->sendPacket(packet);

  while (m_packet.size() == 0) {
    qApp->processEvents(QEventLoop::AllEvents, 100);
  }

  MoleQueue::PacketType refPacket =
      this->readReferenceString("abstractrpcinterface-ref/invalid-method.json");

  QCOMPARE(QString(m_packet), QString(refPacket));
}

void AbstractRpcInterfaceTest::testInvalidParams()
{
  qDebug() << "There is currently no way to trigger an invalid parameter response.";
}

void AbstractRpcInterfaceTest::testInternalError()
{
  qDebug() << "There is currently no way to trigger an internal error response.";
}

QTEST_MAIN(AbstractRpcInterfaceTest)

#include "moc_abstractrpcinterfacetest.cxx"
