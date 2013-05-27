/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2012-2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "openwithactionfactory.h"

#include "../client/jsonrpcclient.h"
#include "../job.h"
#include "../logger.h"
#include "../molequeueconfig.h"
#include "../program.h"
#include "../queue.h"
#include "../queuemanager.h"
#include "../server.h"

#include <QtGui/QAction>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>

#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QVariant>

namespace MoleQueue
{

//==============================================================================
// HandlerStrategy interface
class OpenWithActionFactory::HandlerStrategy
{
public:
  virtual ~HandlerStrategy() {};
  virtual HandlerStrategy* clone() const = 0;
  virtual void readSettings(QSettings &settings) = 0;
  virtual void writeSettings(QSettings &settings) const = 0;
  virtual bool openFile(const QString &fileName, const QString &workDir) = 0;
  virtual QString openFileError() const { return m_error; }
protected:
  QString m_error;
};

namespace {

//==============================================================================
// ExecutableHandlerStrategy implementation
class ExecutableHandlerStrategy : public OpenWithActionFactory::HandlerStrategy
{
  QString m_executable;
public:
  explicit ExecutableHandlerStrategy(const QString &exec = QString());
  HandlerStrategy* clone() const;
  void readSettings(QSettings &settings);
  void writeSettings(QSettings &settings) const;
  bool openFile(const QString &fileName, const QString &workDir);

  QString executable() const { return m_executable; }
};

ExecutableHandlerStrategy::ExecutableHandlerStrategy(const QString &exec)
  : m_executable(exec)
{
}

OpenWithActionFactory::HandlerStrategy* ExecutableHandlerStrategy::clone() const
{
  return new ExecutableHandlerStrategy(m_executable);
}

void ExecutableHandlerStrategy::readSettings(QSettings &settings)
{
  m_executable = settings.value("executable").toString();
}

void ExecutableHandlerStrategy::writeSettings(QSettings &settings) const
{
  settings.setValue("executable", m_executable);
}

bool ExecutableHandlerStrategy::openFile(const QString &fileName,
                                         const QString &workDir)
{
  m_error.clear();

  qint64 pid = -1;
  bool ok = QProcess::startDetached(QString("%1").arg(m_executable),
        QStringList() << fileName, workDir, &pid);

  // pid may be set to zero in certain cases in the UNIX QProcess implementation
  if (ok && pid >= 0) {
    Logger::logDebugMessage(
          Logger::tr("Running '%1 %2' in '%3' (PID=%4)",
                     "1 is an executable, 2 is a filename, 3 is a "
                     "directory, and 4 is a process id.")
          .arg(m_executable).arg(fileName).arg(workDir)
          .arg(QString::number(pid)));
    return true;
  }

  m_error = Logger::tr("Error while starting '%1 %2' in '%3'",
                       "1 is an executable, 2 is a filename, 3 is a directory.")
      .arg(m_executable).arg(fileName).arg(workDir);

  return false;
}

//==============================================================================
// RpcHandlerStrategy implementation
class RpcHandlerStrategy : public OpenWithActionFactory::HandlerStrategy
{
public:
  explicit RpcHandlerStrategy(const QString &server = QString(),
                              const QString &method = QString());
  HandlerStrategy* clone() const;
  void readSettings(QSettings &settings);
  void writeSettings(QSettings &settings) const;
  bool openFile(const QString &fileName, const QString &workDir);

  QString rpcServer() const { return m_rpcServer; }
  QString rpcMethod() const { return m_rpcMethod; }
private:
  QString m_rpcServer;
  QString m_rpcMethod;
};

RpcHandlerStrategy::RpcHandlerStrategy(const QString &server,
                                       const QString &method)
  : m_rpcServer(server),
    m_rpcMethod(method)
{
}

OpenWithActionFactory::HandlerStrategy* RpcHandlerStrategy::clone() const
{
  return new RpcHandlerStrategy(m_rpcServer, m_rpcMethod);
}

void RpcHandlerStrategy::readSettings(QSettings &settings)
{
  m_rpcServer = settings.value("rpcServer").toString();
  m_rpcMethod = settings.value("rpcMethod").toString();
}

void RpcHandlerStrategy::writeSettings(
    QSettings &settings) const
{
  settings.setValue("rpcServer", m_rpcServer);
  settings.setValue("rpcMethod", m_rpcMethod);
}

bool RpcHandlerStrategy::openFile(const QString &fileName,
                                  const QString &workDir)
{
  Q_UNUSED(workDir);

  m_error.clear();
  QEventLoop loop;

  JsonRpcClient client;
  if (!client.connectToServer(m_rpcServer)) {
    m_error = Logger::tr("Unable to connect to RPC server at '%1'.")
        .arg(m_rpcServer);
    return false;
  }
  QObject::connect(&client, SIGNAL(resultReceived(QJsonObject)),
                   &loop, SLOT(quit()));

  QTimer timer;
  timer.setSingleShot(true);
  QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

  QJsonObject req(client.emptyRequest());
  req["method"] = m_rpcMethod;
  QJsonObject params;
  params["fileName"] = fileName;
  req["params"] = params;

  if (!client.sendRequest(req)) {
    m_error = Logger::tr("Cannot send request to RPC server at '%1'")
        .arg(m_rpcServer);
    return false;
  }
  timer.start(3000);
  loop.exec();

  if (!timer.isActive()) {
    m_error = Logger::tr("Timeout waiting for a reply from RPC server '%1'")
        .arg(m_rpcServer);
    return false;
  }

  return true;
}

} // end anon namespace

//==============================================================================
// OpenWithActionFactory implementation:
OpenWithActionFactory::OpenWithActionFactory()
  : JobActionFactory(),
    m_handlerType(NoHandler),
    m_handler(NULL)
{
  qRegisterMetaType<Job>("Job");
  m_isMultiJob = false;
  m_flags |= JobActionFactory::ContextItem;
}

OpenWithActionFactory::~OpenWithActionFactory()
{
  delete m_handler;
}

OpenWithActionFactory::OpenWithActionFactory(const OpenWithActionFactory &other)
  : JobActionFactory(other),
    m_name(other.m_name),
    m_menuText(other.m_menuText),
    m_handlerType(other.m_handlerType),
    m_handler(other.m_handler == NULL ? NULL : other.m_handler->clone()),
    m_filePatterns(other.m_filePatterns),
    m_fileNames(other.m_fileNames)
{
}

OpenWithActionFactory &
OpenWithActionFactory::operator=(OpenWithActionFactory other)
{
  JobActionFactory::operator=(other);

  using namespace std;
  swap(m_handlerType, other.m_handlerType);
  swap(m_handler, other.m_handler);
  swap(m_filePatterns, other.m_filePatterns);
  swap(m_fileNames, other.m_fileNames);

  return *this;
}

void OpenWithActionFactory::readSettings(QSettings &settings)
{
  JobActionFactory::readSettings(settings);

  m_name = settings.value("name").toString();

  setHandlerType(static_cast<HandlerType>(
                   settings.value("handlerType", NoHandler).toInt()));
  settings.beginGroup("handler");
  if (m_handler)
    m_handler->readSettings(settings);
  settings.endGroup();

  m_filePatterns.clear();
  int numPatterns = settings.beginReadArray("patterns");
  for (int i = 0; i < numPatterns; ++i) {
    settings.setArrayIndex(i);
    m_filePatterns << settings.value("regexp").toRegExp();
  }
  settings.endArray();
}

void OpenWithActionFactory::writeSettings(QSettings &settings) const
{
  JobActionFactory::writeSettings(settings);

  settings.setValue("name", m_name);
  settings.setValue("handlerType", m_handlerType);
  settings.beginGroup("handler");
  settings.remove(""); // clear any old handler settings
  if (m_handler)
    m_handler->writeSettings(settings);
  settings.endGroup();

  settings.beginWriteArray("patterns", m_filePatterns.size());
  for (int i = 0; i < m_filePatterns.size(); ++i) {
    settings.setArrayIndex(i);
    settings.setValue("regexp", m_filePatterns[i]);
  }
  settings.endArray();
}

bool OpenWithActionFactory::isValidForJob(const Job &job) const
{
  if (!job.isValid())
    return false;

  QDir directory(job.jobState() == Finished ? job.outputDirectory()
                                            : job.localWorkingDirectory());

  return scanDirectoryForRecognizedFiles(directory, directory);
}

void OpenWithActionFactory::clearJobs()
{
  JobActionFactory::clearJobs();
  m_fileNames.clear();
  m_menuText = QString();
}

bool OpenWithActionFactory::useMenu() const
{
  return true;
}

QString OpenWithActionFactory::menuText() const
{
  return m_menuText;
}

QList<QAction *> OpenWithActionFactory::createActions()
{
  QList<QAction*> result;

  if (m_attemptedJobAdditions == 1 && m_jobs.size() == 1) {
    const Job &job = m_jobs.first();
    m_menuText = tr("Open '%1' with %2")
        .arg(job.description(), name());
    QStringList fileNames = m_fileNames.keys();
    foreach (const QString &fileName, fileNames) {
      QAction *newAction = new QAction(fileName, NULL);
      newAction->setData(QVariant::fromValue(job));
      newAction->setProperty("fileName", m_fileNames.value(fileName));
      connect(newAction, SIGNAL(triggered()), this, SLOT(actionTriggered()));
      result << newAction;
    }
  }

  return result;
}

unsigned int OpenWithActionFactory::usefulness() const
{
  return 800;
}

void OpenWithActionFactory::setHandlerType(
    OpenWithActionFactory::HandlerType type)
{
  if (type == m_handlerType)
    return;

  delete m_handler;
  switch (m_handlerType = type) {
  case ExecutableHandler:
    m_handler = new ExecutableHandlerStrategy;
    break;
  case RpcHandler:
    m_handler = new RpcHandlerStrategy;
    break;
  default:
  case NoHandler:
    m_handler = NULL;
    break;
  }
}

void OpenWithActionFactory::setExecutable(const QString &exec)
{
  delete m_handler;
  m_handler = new ExecutableHandlerStrategy(exec);
  m_handlerType = ExecutableHandler;
}

QString OpenWithActionFactory::executable() const
{
  return m_handlerType == ExecutableHandler
      ? static_cast<ExecutableHandlerStrategy*>(m_handler)->executable()
      : QString();
}

void OpenWithActionFactory::setRpcDetails(const QString &myRpcServer,
                                          const QString &myRpcMethod)
{
  delete m_handler;
  m_handler = new RpcHandlerStrategy(myRpcServer, myRpcMethod);
  m_handlerType = RpcHandler;
}

QString OpenWithActionFactory::rpcServer() const
{
  return m_handlerType == RpcHandler
      ? static_cast<RpcHandlerStrategy*>(m_handler)->rpcServer()
      : QString();
}

QString OpenWithActionFactory::rpcMethod() const
{
  return m_handlerType == RpcHandler
      ? static_cast<RpcHandlerStrategy*>(m_handler)->rpcMethod()
      : QString();
}

QList<QRegExp> OpenWithActionFactory::filePatterns() const
{
  return m_filePatterns;
}

QList<QRegExp> &OpenWithActionFactory::filePatternsRef()
{
  return m_filePatterns;
}

const QList<QRegExp> &OpenWithActionFactory::filePatternsRef() const
{
  return m_filePatterns;
}

void OpenWithActionFactory::setFilePatterns(const QList<QRegExp> &patterns)
{
  m_filePatterns = patterns;
}

void OpenWithActionFactory::actionTriggered()
{
  QAction *action = qobject_cast<QAction*>(sender());
  if (!action) {
    Logger::logWarning(tr("OpenWithActionFactory::actionTriggered: Sender is "
                          "not a QAction!"));
    return;
  }

  // The sender was a QAction. Is its data a job?
  Job job = action->data().value<Job>();
  if (!job.isValid()) {
    Logger::logWarning(tr("OpenWithActionFactory::actionTriggered: Action data "
                          "is not a Job."));
    return;
  }

  // Filename was set?
  QString fileName = action->property("fileName").toString();
  if (!QFileInfo(fileName).exists()) {
    Logger::logWarning(tr("OpenWithActionFactory::actionTriggered: No filename "
                          "associated with job."), job.moleQueueId());
    return;
  }

  // Is the handler set?
  if (!m_handler) {
    Logger::logWarning(tr("OpenWithActionFactory::actionTriggered: No handler "
                          "set."), job.moleQueueId());
    return;
  }

  // Setup the progress dialog
  QProgressDialog dialog;
  dialog.setMinimumDuration(500);
  dialog.setRange(0, 0);
  dialog.setValue(0);

  // Should be ready to go!
  QString workDir = QFileInfo(fileName).absolutePath();
  if (!m_handler->openFile(fileName, workDir)) {
    QString err(tr("Error: %1").arg(m_handler->openFileError()));
    Logger::logWarning(err, job.moleQueueId());
    QMessageBox::critical(NULL, tr("Cannot start process"), err);
  }
}

bool OpenWithActionFactory::scanDirectoryForRecognizedFiles(
    const QDir &baseDir, const QDir &dir) const
{
  bool result = false;

  // Recursively check subdirectories
  QStringList subDirs(dir.entryList(QDir::Dirs | QDir::Readable |
                                    QDir::NoDotAndDotDot, QDir::Name));
  foreach (const QString &subDir, subDirs) {
    if (scanDirectoryForRecognizedFiles(baseDir,
                                        QDir(dir.absoluteFilePath(subDir))))
      result = true;
  }

  QStringList entries(dir.entryList(QDir::Files | QDir::Readable,
                                    QDir::Name));

  foreach (const QString &fileName, entries) {
    foreach (const QRegExp &regexp, m_filePatterns) {
      if (regexp.indexIn(fileName) >= 0) {
        result = true;
        m_fileNames.insert(
              baseDir.relativeFilePath(dir.absoluteFilePath(fileName)),
              dir.absoluteFilePath(fileName));
      }
    }
  }

  return result;
}

} // end namespace MoleQueue
