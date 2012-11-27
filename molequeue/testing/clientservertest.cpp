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

#include "filesystemtools.h"

#include "molequeuetestconfig.h"
#include "testserver.h" // for getRandomSocketName

#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>

// Define ENABLE_ZMQ_TESTS if both zeromq and python are available
#ifndef MoleQueue_PYTHON_EXECUTABLE
#ifndef MoleQueue_HAS_ZMQ
#define ENABLE_ZMQ_TESTS
#endif // MoleQueue_HAS_ZMQ
#endif // MoleQueue_PYTHON_EXECUTABLE

class ClientServerTest : public QObject
{
  Q_OBJECT
public:
  ClientServerTest()
    : QObject(NULL),
      m_numClients(10),
      m_workDir(MoleQueue_BINARY_DIR "/testworkdir"),
      m_moleQueueExecutable(MoleQueue_BINARY_DIR "/bin/molequeue"),
      m_serverProcess(NULL)
  {
#ifdef __APPLE__
    m_moleQueueExecutable = MoleQueue_BINARY_DIR "/bin/molequeue.app/Contents/"
        "MacOS/molequeue";
#endif // __APPLE__
    randomizeSocketName();
  }

private:
  int m_numClients;
  QString m_workDir;
  QString m_socketName;
  QString m_moleQueueExecutable;
  QStringList m_moleQueueDefaultArgs;
  QProcess *m_serverProcess;
  QList<QProcess*> m_clientProcesses;

  /// Delete the testing workdir and initialize it with the directory at
  /// @a sourcePath.
  bool resetWorkDir(const QString &sourcePath);

  /// Create a new randomized socket name, stored in m_socketName;
  void randomizeSocketName();

  /// Create the server process (m_serverProcess) and reset
  /// m_moleQueueDefaultArgs to set the workdir, socketname, and enable rpcKill.
  bool setupServerProcess();

  /// Create a Cxx client process
  QProcess *addClientProcess();

#ifdef ENABLE_ZMQ_TESTS
  /// Create a client process initialized for python. The process is returned
  /// and added to m_clientProcesses.
  QProcess *addPythonClientProcess();
#endif // ENABLE_ZMQ_TESTS

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  // Python client tests:
#ifdef ENABLE_ZMQ_TESTS
  void submitOnePy();
  void submit200Py();
  void submit200FromManyClientsPy();
#endif // ENABLE_ZMQ_TESTS
};


bool ClientServerTest::resetWorkDir(const QString &sourcePath)
{
  // Initialize working directory
  if (QFileInfo(m_workDir).exists()) {
    if (!MoleQueue::FileSystemTools::recursiveRemoveDirectory(m_workDir)) {
      qWarning() << "Could not remove old working directory" << m_workDir;
      return false;
    }
  }
  if (!MoleQueue::FileSystemTools::recursiveCopyDirectory(sourcePath,
                                                          m_workDir)) {
    qWarning() << "Could not initialize working directory" << m_workDir
               << "from" << sourcePath;
    return false;
  }
  return true;
}

void ClientServerTest::randomizeSocketName()
{
  m_socketName = TestServer::getRandomSocketName();
}

bool ClientServerTest::setupServerProcess()
{
  m_moleQueueDefaultArgs.clear();
  m_moleQueueDefaultArgs
      << "--workdir" << m_workDir
      << "--socketname" << m_socketName
      << "--rpc-kill";

  if (m_serverProcess) {
    delete m_serverProcess;
    m_serverProcess = NULL;
  }
  m_serverProcess = new QProcess(this);
  m_serverProcess->setProcessChannelMode(QProcess::ForwardedChannels);
  return true;
}

QProcess *ClientServerTest::addClientProcess()
{
  QProcess *clientProcess = new QProcess(this);
  clientProcess->setProcessChannelMode(::QProcess::ForwardedChannels);
  m_clientProcesses.append(clientProcess);
  return clientProcess;
}

#ifdef ENABLE_ZMQ_TESTS
QProcess *ClientServerTest::addPythonClientProcess()
{
  QProcess *clientProcess = addClientProcess();
  QProcessEnvironment env = clientProcess->processEnvironment();
  env.insert("PYTHONPATH", (env.value("PYTHONPATH").isEmpty()
                            ? QString()
                            : env.value("PYTHONPATH") + ':') +
             MoleQueue_SOURCE_DIR "/python");
  clientProcess->setProcessEnvironment(env);
  return clientProcess;
}
#endif // ENABLE_ZMQ_TESTS

void ClientServerTest::initTestCase()
{
  QVERIFY2(resetWorkDir(MoleQueue_TESTDATA_DIR "/testworkdir_unix"),
           "Failed to reset working directory for test.");

  // Setup server process
  QVERIFY(setupServerProcess());

  // Start server
  qDebug() << "Starting server:" << m_moleQueueExecutable
           << m_moleQueueDefaultArgs.join(" ");
  m_serverProcess->start(m_moleQueueExecutable, m_moleQueueDefaultArgs);
  QVERIFY(m_serverProcess->waitForStarted(10*1000));
  QTest::qSleep(1 * 1000); // Wait one second for server to start
}

void ClientServerTest::cleanupTestCase()
{
  // send killRpc message
  QProcess *clientProcess = addClientProcess();
  QString clientCommand = MoleQueue_TESTEXEC_DIR "sendRpcKill";
  QStringList clientArguments;
  clientArguments << "-s" << m_socketName;

  qDebug() << "Starting client:" << clientCommand
           << clientArguments.join(" ");
  clientProcess->start(clientCommand, clientArguments);

  // Wait for client to finish
  QVERIFY2(clientProcess->waitForFinished(300*1000), "Client timed out.");
  QCOMPARE(clientProcess->exitCode(), 0);

  // Wait for server to finish
  QVERIFY2(m_serverProcess->waitForFinished(5*1000), "Server timed out.");
  QCOMPARE(m_serverProcess->exitCode(), 0);

  // In case the rpcKill call fails, kill the process
  if (m_serverProcess->state() != QProcess::NotRunning)
    m_serverProcess->kill();
  m_serverProcess->deleteLater();
  m_serverProcess = NULL;

  // Clean up the sendRpcKill client.
  cleanup();
}

void ClientServerTest::init()
{
}

void ClientServerTest::cleanup()
{
  foreach (QProcess *proc, m_clientProcesses) {
    if (proc->state() != QProcess::NotRunning)
      proc->kill();
  }
  qDeleteAll(m_clientProcesses);
  m_clientProcesses.clear();
}

#ifdef ENABLE_ZMQ_TESTS
void ClientServerTest::submitOnePy()
{
  // Setup client process
  QProcess *clientProcess = addPythonClientProcess();
  QString clientCommand = MoleQueue_PYTHON_EXECUTABLE;
  QStringList clientArguments;
  clientArguments
      << MoleQueue_TESTSCRIPT_DIR "/submitJob.py"
      << "-s" << m_socketName
      << "-n" << QString::number(1);

  qDebug() << "Starting client:" << clientCommand
           << clientArguments.join(" ");

  clientProcess->start(clientCommand, QStringList(clientArguments));

  // Wait 5 seconds for client to start
  QVERIFY2(clientProcess->waitForStarted(5*1000), "Client did not start.");

  // Wait 10 seconds for client to finish
  QVERIFY2(clientProcess->waitForFinished(10*1000), "Client timed out.");
  QCOMPARE(clientProcess->exitCode(), 0);
}

void ClientServerTest::submit200Py()
{
  // Setup client process
  QProcess *clientProcess = addPythonClientProcess();
  QString clientCommand = MoleQueue_PYTHON_EXECUTABLE;
  QStringList clientArguments;
  clientArguments
      << MoleQueue_TESTSCRIPT_DIR "/submitJob.py"
      << "-s" << m_socketName
      << "-n" << QString::number(200);

  qDebug() << "Starting client:" << clientCommand
           << clientArguments.join(" ");

  clientProcess->start(clientCommand, QStringList(clientArguments));

  // Wait 5 seconds for client to start
  QVERIFY2(clientProcess->waitForStarted(5*1000), "Client did not start.");

  // Wait one minute for client to finish
  QVERIFY2(clientProcess->waitForFinished(60*1000), "Client timed out.");
  QCOMPARE(clientProcess->exitCode(), 0);
}

void ClientServerTest::submit200FromManyClientsPy()
{
  // Setup client processes
  while (m_clientProcesses.size() < m_numClients)
    addPythonClientProcess();
  QString clientCommand = MoleQueue_PYTHON_EXECUTABLE;
  QStringList clientArguments;
  clientArguments
      << MoleQueue_TESTSCRIPT_DIR "/submitJob.py"
      << "-s" << m_socketName
      << "-n" << QString::number(200);

  qDebug() << "Starting" << m_numClients << "clients:" << clientCommand
           << clientArguments.join(" ");

  int clientId = 0;
  foreach (QProcess *cliProc, m_clientProcesses) {
    cliProc->start(clientCommand,
                   QStringList(clientArguments)
                   << "-c" << QString::number(++clientId));
  }

  // Wait 5 seconds for each client to start
  clientId = 0;
  foreach (QProcess *cliProc, m_clientProcesses) {
    ++clientId;
    QVERIFY2(cliProc->waitForStarted(5*1000),
             (QByteArray("Client ") + QByteArray::number(clientId) +
              QByteArray(" failed to start.")).constData());
  }

  // Wait two minutes for all clients to finish
  clientId = 0;
  foreach (QProcess *cliProc, m_clientProcesses) {
    ++clientId;
    QVERIFY2(cliProc->waitForFinished(2*60*1000),
             (QByteArray("Client ") + QByteArray::number(clientId) +
              QByteArray(" timed out.")).constData());
    QCOMPARE(cliProc->exitCode(), 0);
  }
}
#endif // ENABLE_ZMQ_TESTS
QTEST_MAIN(ClientServerTest)

#include "clientservertest.moc"
