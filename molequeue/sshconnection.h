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

#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include <QtCore/QObject>

namespace MoleQueue {

class SshConnection : public QObject
{
  Q_OBJECT

public:
  SshConnection(QObject *parentObject = 0);
  ~SshConnection();

  /** \return If the SSH connection is set as persistent or not. */
  bool isPersistent() const { return m_persistent; }

  /** \return The user name that will be used. */
  QString userName() const { return m_userName; }

  /** \return The host that will be used. */
  QString hostName() const { return m_hostName; }

  /** \return The path to the identity file that will be used. */
  QString identityFile() const { return m_identityFile; }

  /** \return The port that will be used. */
  int portNumber() const { return m_portNumber; }

  /** \return Whether the connection is valid, at a minimum need a host name. */
  bool isValid() const;

public slots:
  /**
   * Set whether the connection should be persistent, or each issuesd command
   * uses a short-lived connection, e.g. on the command line a non-persistent
   * connection would be the equivalent of,
   *
   * ssh user@host ls
   */
  void setPersistent(bool persist) { m_persistent = persist; }

  /**
   * Set the user name to use for the connection.
   */
  void setUserName(const QString &newUserName) { m_userName = newUserName; }

  /**
   * Set the host name to use for the connection.
   */
  void setHostName(const QString &newHostName) { m_hostName = newHostName; }

  /**
   * Set the identity file to use for the connection. This is the path to the
   * private key to be used when establishing the connection.
   */
  void setIdentityFile(const QString &newIdentityFile)
  {
    m_identityFile = newIdentityFile;
  }

  /**
   * Set the host name to use for the connection.
   */
  void setPortNumber(int newPortNumber) { m_portNumber = newPortNumber; }

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
  bool m_persistent;
  QString m_userName;
  QString m_hostName;
  QString m_identityFile;
  int m_portNumber;
};

} // End of namespace

#endif
