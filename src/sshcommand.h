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

#ifndef SSHCOMMAND_H
#define SSHCOMMAND_H

#include "sshconnection.h"

#include <QtCore/QStringList>

namespace MoleQueue {

class TerminalProcess;

/**
 * Concrete implementation of the SshConnection class using the command line
 * SSH command. It is executed using TerminalProcess.
 */

class SshCommand : public SshConnection
{
  Q_OBJECT

public:
  SshCommand(QObject *parent = 0);
  ~SshCommand();

  /** \return The SSH command that will be run. */
  QString sshCommand() { return m_sshCommand; }

  /** \return The SCP command that will be run. */
  QString scpCommand() { return m_scpCommand; }

public slots:
  /**
   * Set the SSH command for the class. Defaults to 'ssh', and would execute
   * the SSH commnand in the user's path.
   */
  void setSshCommand(const QString &command) { m_sshCommand = command; }

  /**
   * Set the SCP command for the class. Defaults to 'scp', and would execute
   * the SCP commnand in the user's path.
   */
  void setScpCommand(const QString &command) { m_scpCommand = command; }

  /**
   * Execute the supplied command on the remote host.
   * \param command The command to execute.
   * \param output The output from the command (if any).
   * \param exitCode The exit code from the command.
   * \return True on success, false on failure.
   */
  virtual bool execute(const QString &command, QString &output, int &exitCode);

  /**
   * Copy a local file to the remote system.
   * \param localFile The path of the local file.
   * \param remoteFile The path of the file on the remote system.
   * \return True on success, false on failure.
   */
  virtual bool copyTo(const QString &localFile, const QString &remoteFile);

  /**
   * Copy a remote file to the local system.
   * \param remoteFile The path of the file on the remote system.
   * \param localFile The path of the local file.
   * \return True on success, false on failure.
   */
  virtual bool copyFrom(const QString &remoteFile, const QString &localFile);

  /**
   * Copy a local directory recursively to the remote system.
   * \param localDir The path of the local directory.
   * \param remoteDir The path of the directory on the remote system.
   * \return True on success, false on failure.
   */
  virtual bool copyDirTo(const QString &localDir, const QString &remoteDir);

  /**
   * Copy a remote directory recursively to the local system.
   * \param remoteDir The path of the directory on the remote system.
   * \param localFile The path of the local directory.
   * \return True on success, false on failure.
   */
  virtual bool copyDirFrom(const QString &remoteDir, const QString &localDir);

protected:
  QString m_sshCommand;
  QString m_scpCommand;
  TerminalProcess *m_process;

  /** Initialize the TerminalProcess object. */
  void initializeProcess();

  /** Return the arguments to be passed to the SCP command. */
  QStringList scpArgs();
};

} // End namespace

#endif
