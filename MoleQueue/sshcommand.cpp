/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

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

SshCommand::SshCommand(QObject *parent) : SshConnection(parent),
  m_sshCommand("ssh"),
  m_scpCommand("scp"),
  m_process(0)
{
}

SshCommand::~SshCommand()
{
  delete m_process;
  m_process = 0;
}

bool SshCommand::execute(const QString &command, QString &output, int &exitCode)
{
  if (!isValid())
    return false;
  if (!m_process)
    initializeProcess();

  QStringList args;
  if (!m_userName.isEmpty())
    args << "-l" << m_userName;
  if (!m_identityFile.isEmpty())
    args << "-i" << m_identityFile;
  if (m_portNumber >= 0)
    args << "-p" << QString(m_portNumber);
  // Finally append the host name and the actual command
  args << m_hostName << command;

  m_process->start(m_sshCommand, args);
  if (!m_process->waitForStarted()) {
    qDebug() << "Failed to start SSH command...";
    return false;
  }
  m_process->closeWriteChannel();
  if (!m_process->waitForFinished()) {
    m_process->close();
    qDebug() << "Failed to exit.";
    return false;
  }
  output = m_process->readAll();
  qDebug() << "SSH command:" << output;
  exitCode = m_process->exitCode();
  m_process->close();
  return true;
}

bool SshCommand::copyTo(const QString &localFile, const QString &remoteFile)
{
  if (!isValid())
    return false;
  if (!m_process)
    initializeProcess();

  QStringList args = scpArgs();
//  args << "-v";
  QString remoteFileSpec;
  if (m_userName.isEmpty())
    remoteFileSpec = m_hostName;
  else
    remoteFileSpec = m_userName + "@" + m_hostName;
  remoteFileSpec += ":" + remoteFile;

  // Build up the command line with local and remote files
  args << localFile << remoteFileSpec;

  qDebug() << m_scpCommand << args;

  m_process->start(m_scpCommand, args);
  if (!m_process->waitForStarted()) {
    qDebug() << "Failed to start SCP command...";
    return false;
  }
  m_process->closeWriteChannel();
  if (!m_process->waitForFinished()) {
    m_process->close();
    qDebug() << "Failed to exit.";
    return false;
  }
  QString output = m_process->readAll();
  qDebug() << "Output:" << output;
  int exitCode = m_process->exitCode();
  qDebug() << "Exit code:" << exitCode;

  return true;
}

bool SshCommand::copyFrom(const QString &remoteFile, const QString &localFile)
{
  if (!isValid())
    return false;
  if (!m_process)
    initializeProcess();

  QStringList args = scpArgs();
  args << "-v";
  QString remoteFileSpec;
  if (m_userName.isEmpty())
    remoteFileSpec = m_hostName;
  else
    remoteFileSpec = m_userName + "@" + m_hostName;
  remoteFileSpec += ":" + remoteFile;

  // Build up the command line with local and remote files
  args << remoteFileSpec << localFile;

  qDebug() << m_scpCommand << args;

  m_process->start(m_scpCommand, args);
  if (!m_process->waitForStarted()) {
    qDebug() << "Failed to start SCP command...";
    return false;
  }
  m_process->closeWriteChannel();
  if (!m_process->waitForFinished()) {
    m_process->close();
    qDebug() << "Failed to exit.";
    return false;
  }
  QString output = m_process->readAll();
  qDebug() << "Output:" << output;
  int exitCode = m_process->exitCode();
  qDebug() << "Exit code:" << exitCode;

  return true;
}

bool SshCommand::copyDirTo(const QString &localDir, const QString &remoteDir)
{
  if (!isValid())
    return false;
  if (!m_process)
    initializeProcess();

  QStringList args = scpArgs();
  args << "-r";
  QString remoteSpec;
  if (m_userName.isEmpty())
    remoteSpec = m_hostName;
  else
    remoteSpec = m_userName + "@" + m_hostName;
  remoteSpec += ":" + remoteDir;

  // Build up the command line with local and remote files
  args << localDir << remoteSpec;

  qDebug() << m_scpCommand << " " << args.join(" ");

  m_process->start(m_scpCommand, args);
  if (!m_process->waitForStarted()) {
    qDebug() << "Failed to start SCP command...";
    return false;
  }
  m_process->closeWriteChannel();
  if (!m_process->waitForFinished()) {
    m_process->close();
    qDebug() << "Failed to exit.";
    return false;
  }
  QString output = m_process->readAll();
  qDebug() << "Output:" << output;
  int exitCode = m_process->exitCode();
  qDebug() << "Exit code:" << exitCode;

  return true;
}

bool SshCommand::copyDirFrom(const QString &remoteDir, const QString &localDir)
{
  if (!isValid())
    return false;
  if (!m_process)
    initializeProcess();

  QDir local(localDir);
  if (!local.exists())
    local.mkpath(localDir);

  QStringList args = scpArgs();
  args << "-r";
  QString remoteSpec;
  if (m_userName.isEmpty())
    remoteSpec = m_hostName;
  else
    remoteSpec = m_userName + "@" + m_hostName;
  remoteSpec += ":" + remoteDir;

  // Build up the command line with local and remote files
  args << remoteSpec << localDir;

  qDebug() << m_scpCommand << " " << args.join(" ");

  m_process->start(m_scpCommand, args);
  if (!m_process->waitForStarted()) {
    qDebug() << "Failed to start SCP command...";
    return false;
  }
  m_process->closeWriteChannel();
  if (!m_process->waitForFinished()) {
    m_process->close();
    qDebug() << "Failed to exit.";
    return false;
  }
  QString output = m_process->readAll();
  qDebug() << "Output:" << output;
  int exitCode = m_process->exitCode();
  qDebug() << "Exit code:" << exitCode;

  return true;
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
}

QStringList SshCommand::scpArgs()
{
  QStringList args;
  if (!m_identityFile.isEmpty())
    args << "-i" << m_identityFile;
  if (m_portNumber >= 0)
    args << "-P" << QString(m_portNumber);
  return args;
}

} // End namespace
