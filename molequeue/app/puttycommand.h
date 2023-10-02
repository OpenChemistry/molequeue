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

#ifndef PUTTYCOMMAND_H
#define PUTTYCOMMAND_H

#include "sshcommand.h"

namespace MoleQueue {

class TerminalProcess;


/**
 * @class PuttyCommand puttycommand.h <molequeue/puttycommand.h>
 * @brief Concrete implementation of SshCommand using commandline plink/pscp.
 * @author Marcus D. Hanwell, Allison Vacanti, Chris Harris
 *
 * The PuttyCommand provides an implementation of the SshCommand interface
 * that calls the commandline plink and pscp executables in a TerminalProcess.
 *
 * When writing code that needs ssh functionality, the code should use the
 * SshConnection interface instead.
 */
class PuttyCommand : public SshCommand
{
  Q_OBJECT

public:
  PuttyCommand(QObject *parentObject = 0);
  ~PuttyCommand();

protected:

  /// @return the arguments to be passed to the SSH command.
  QStringList sshArgs();

  /// @return the arguments to be passed to the SCP command.
  QStringList scpArgs();
};

} // End namespace

#endif
