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

#ifndef DUMMYSSHCOMMAND_H
#define DUMMYSSHCOMMAND_H

#include "sshcommand.h"

/// SshCommand implementation that doesn't actually call external processes.
class DummySshCommand : public MoleQueue::SshCommand
{
  Q_OBJECT
public:
  DummySshCommand(QObject *parentObject = NULL);

  ~DummySshCommand();

  QString getDummyCommand() const { return m_dummyCommand; }
  QStringList getDummyArgs() const { return m_dummyArgs; }
  void setDummyOutput(const QString &out) { m_output = out; }
  void setDummyExitCode(int code) {m_exitCode = code; }
  void emitDummyRequestComplete() { emit requestComplete(); }

protected:
  void sendRequest(const QString &command, const QStringList &args)
  {
    m_dummyCommand = command;
    m_dummyArgs = args;
  }

  QString m_dummyCommand;
  QStringList m_dummyArgs;
};

#endif // DUMMYSSHCOMMAND_H
