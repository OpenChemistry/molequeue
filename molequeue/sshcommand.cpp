/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "sshcommand.h"
#include "terminalprocess.h"
#include <QtCore/QProcessEnvironment>
#include <QtCore/QDir>
#include <QtCore/QDebug>

namespace MoleQueue {

SshCommand::SshCommand(QObject *parentObject) : SshConnection(parentObject),
  m_sshCommand("ssh"),
  m_scpCommand("scp"),
  m_exitCode(-1),
  m_process(0),
  m_isComplete(true)
{
}

SshCommand::~SshCommand()
{
  delete m_process;
  m_process = 0;
}

QString SshCommand::output() const
{
  return isComplete() ? m_output : QString();
}

int SshCommand::exitCode() const
{
  return isComplete() ? m_exitCode : -1;
}

bool SshCommand::waitForCompletion(int msecs)
{
  if (!m_process)
    return false;

  if (m_process->state() == QProcess::Starting)
    m_process->waitForStarted(msecs);

  if (m_isComplete)
    return true;

  return m_process->waitForFinished(msecs);
}

bool SshCommand::isComplete() const
{
  return m_isComplete;
}

bool SshCommand::execute(const QString &command)
{
  if (!isValid())
    return false;

  QStringList args = sshArgs();
  args << remoteSpec() << command;

  sendRequest(m_sshCommand, args);

  return true;
}

bool SshCommand::copyTo(const QString &localFile, const QString &remoteFile)
{
  if (!isValid())
    return false;

  QStringList args = scpArgs();
  QString remoteFileSpec = remoteSpec() + ":" + remoteFile;
  args << localFile << remoteFileSpec;

  sendRequest(m_scpCommand, args);

  return true;
}

bool SshCommand::copyFrom(const QString &remoteFile, const QString &localFile)
{
  if (!isValid())
    return false;

  QStringList args = scpArgs();
  QString remoteFileSpec = remoteSpec() + ":" + remoteFile;
  args << remoteFileSpec << localFile;

  sendRequest(m_scpCommand, args);

  return true;
}

bool SshCommand::copyDirTo(const QString &localDir, const QString &remoteDir)
{
  if (!isValid())
    return false;

  QStringList args = scpArgs();
  QString remoteDirSpec = remoteSpec() + ":" + remoteDir;
  args << "-r" << localDir << remoteDirSpec;

  sendRequest(m_scpCommand, args);

  return true;
}

bool SshCommand::copyDirFrom(const QString &remoteDir, const QString &localDir)
{
  if (!isValid())
    return false;

  QDir local(localDir);
  if (!local.exists())
    local.mkpath(localDir); /// @todo Check for failure of mkpath

  QStringList args = scpArgs();
  QString remoteDirSpec = remoteSpec() + ":" + remoteDir;
  args << "-r" << remoteDirSpec << localDir;

  sendRequest(m_scpCommand, args);

  return true;
}

void SshCommand::processStarted()
{
  m_process->closeWriteChannel();
  emit requestSent();
}

void SshCommand::processFinished()
{
  m_output = m_process->readAll();
  m_exitCode = m_process->exitCode();
  m_process->close();

  qDebug() << "SshCommand exit:" << m_exitCode << "output:\n" << m_output;

  m_isComplete = true;
  emit requestComplete();
}

void SshCommand::sendRequest(const QString &command, const QStringList &args)
{
  if (!m_process)
    initializeProcess();

  m_isComplete = false;

  qDebug() << "SshCommand sending request:" << command << args.join(" ");

  m_process->start(command, args);
}

void SshCommand::initializeProcess()
{
  // Initialize the environment for the process, set merged channels.
  if (!m_process)
    m_process = new TerminalProcess(this);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QProcessEnvironment sshEnv;
  if (env.contains("DISPLAY"))
    sshEnv.insert("DISPLAY", env.value("DISPLAY"));
  if (env.contains("EDITOR"))
    sshEnv.insert("EDITOR", env.value("EDITOR"));
  if (env.contains("SSH_AUTH_SOCK"))
    sshEnv.insert("SSH_AUTH_SOCK", env.value("SSH_AUTH_SOCK"));
  sshEnv.insert("SSH_ASKPASS", "/usr/bin/pinentry-qt4");
  m_process->setProcessEnvironment(sshEnv);
  m_process->setProcessChannelMode(QProcess::MergedChannels);

  connect(m_process, SIGNAL(started()), this, SLOT(processStarted()));
  connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
          this, SLOT(processFinished()));
}

QStringList SshCommand::sshArgs()
{
  QStringList args;
  // Suppress login banners
  args << "-q";
  if (!m_identityFile.isEmpty())
    args << "-i" << m_identityFile;
  if (m_portNumber >= 0)
    args << "-p" << QString::number(m_portNumber);
  return args;
}

QStringList SshCommand::scpArgs()
{
  QStringList args;
  // Suppress login banners
  args << "-q";
  if (!m_identityFile.isEmpty())
    args << "-i" << m_identityFile;
  if (m_portNumber >= 0)
    args << "-P" << QString::number(m_portNumber);
  return args;
}

QString SshCommand::remoteSpec()
{
  return m_userName.isEmpty() ? m_hostName : m_userName + "@" + m_hostName;
}

} // End namespace
