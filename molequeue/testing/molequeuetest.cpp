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

#include "testserver.h" // for getRandomSocketName

#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>

class MoleQueueTest : public QObject
{
  Q_OBJECT
public:
  MoleQueueTest()
    : QObject(NULL),
      m_numClients(5),
      m_workDir(MoleQueue_BINARY_DIR "/testworkdir"),
      m_moleQueueExecutable(MoleQueue_BINARY_DIR "/bin/molequeue"),
      m_serverProcess(NULL)
  {
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

  /// Create a client process initialized for python. The process is returned
  /// and added to m_clientProcesses.
  QProcess *addPythonClientProcess();

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void runPythonTests();
};


bool MoleQueueTest::resetWorkDir(const QString &sourcePath)
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

void MoleQueueTest::randomizeSocketName()
{
  m_socketName = TestServer::getRandomSocketName();
}

bool MoleQueueTest::setupServerProcess()
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

QProcess *MoleQueueTest::addPythonClientProcess()
{
  QProcess *clientProcess = new QProcess(this);
  QProcessEnvironment env = clientProcess->processEnvironment();
  env.insert("PYTHONPATH", (env.value("PYTHONPATH").isEmpty()
                            ? QString()
                            : env.value("PYTHONPATH") + ':') +
             MoleQueue_SOURCE_DIR "/python");
  clientProcess->setProcessEnvironment(env);
  clientProcess->setProcessChannelMode(QProcess::ForwardedChannels);
  m_clientProcesses.append(clientProcess);
  return clientProcess;
}

void MoleQueueTest::initTestCase()
{
}

void MoleQueueTest::cleanupTestCase()
{
}

void MoleQueueTest::init()
{
}

void MoleQueueTest::cleanup()
{
  foreach (QProcess *proc, m_clientProcesses) {
    if (proc->state() != QProcess::NotRunning)
      proc->kill();
  }
  qDeleteAll(m_clientProcesses);
  m_clientProcesses.clear();
  if (m_serverProcess->state() != QProcess::NotRunning)
    m_serverProcess->kill();
  m_serverProcess->deleteLater();
  m_serverProcess = NULL;
}

void MoleQueueTest::runPythonTests()
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

  // Setup client process
  QString clientCommand = MoleQueue_PYTHON_EXECUTABLE;
  QStringList clientArguments;
  clientArguments
      << ""
      << "-s" << m_socketName;
  QString &scriptName = clientArguments[0];
  QProcess *clientProcess = addPythonClientProcess();

  /**************** submitOne.py **********************/

  // Start client
  scriptName = MoleQueue_TESTSCRIPT_DIR "/submitOne.py";
  qDebug() << "Starting client:" << clientCommand
           << clientArguments.join(" ");
  clientProcess->start(clientCommand, clientArguments);

  // Wait for client to finish
  QVERIFY2(clientProcess->waitForFinished(300*1000), "Client timed out.");
  QCOMPARE(clientProcess->exitCode(), 0);

  /**************** submit200.py **********************/

  // Start client
  scriptName = MoleQueue_TESTSCRIPT_DIR "/submit200.py";
  qDebug() << "Starting client:" << clientCommand
           << clientArguments.join(" ");
  clientProcess->start(clientCommand, clientArguments);

  // Wait for client to finish
  QVERIFY2(clientProcess->waitForFinished(300*1000), "Client timed out.");
  QCOMPARE(clientProcess->exitCode(), 0);

  /**************** multiple submit200.py **********************/

  // Create clients
  while (m_clientProcesses.size() < m_numClients)
    addPythonClientProcess();

  // Start client
  scriptName = MoleQueue_TESTSCRIPT_DIR "/submit200.py";
  qDebug() << "Starting" << m_numClients << "clients:" << clientCommand
           << clientArguments.join(" ");
  int clientId = 0;
  foreach (QProcess *cliProc, m_clientProcesses) {
    cliProc->start(clientCommand,
                   QStringList(clientArguments)
                   << "-c" << QString::number(++clientId));
  }

  // Wait for clients to finish
  clientId = 0;
  foreach (QProcess *cliProc, m_clientProcesses) {
    qDebug() << "Waiting for client" << ++clientId << "state:" << cliProc->state();
    QVERIFY2(cliProc->waitForFinished(300*1000), "Client timed out.");
    QCOMPARE(cliProc->exitCode(), 0);
  }

  /***************** Server cleanup *******************/

  // send killRpc message
  scriptName = MoleQueue_TESTSCRIPT_DIR "/sendRpcKill.py";
  qDebug() << "Starting client:" << clientCommand
           << clientArguments.join(" ");
  clientProcess->start(clientCommand, clientArguments);

  // Wait for client to finish
  QVERIFY2(clientProcess->waitForFinished(300*1000), "Client timed out.");
  QCOMPARE(clientProcess->exitCode(), 0);

  // Wait for server to finish
  QVERIFY2(m_serverProcess->waitForFinished(5*1000), "Server timed out.");
  QCOMPARE(m_serverProcess->exitCode(), 0);
}

QTEST_MAIN(MoleQueueTest)

#include "molequeuetest.moc"
