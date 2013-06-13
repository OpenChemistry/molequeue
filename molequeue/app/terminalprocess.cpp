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

#include "terminalprocess.h"

#ifdef Q_OS_UNIX
# include <unistd.h>
#endif

namespace MoleQueue {

TerminalProcess::TerminalProcess(QObject *parentObject) :
  QProcess(parentObject)
{
}

TerminalProcess::~TerminalProcess()
{
}

void TerminalProcess::setupChildProcess()
{
#ifdef Q_OS_UNIX
  // Become the session leader on Unix (no-op on Windows). This makes things
  // like SSH use GUIs to prompt for passwords (SSH_ASKPASS) as there is no
  // tty associated with the process.
  setsid();
#endif
}

} // End namespace
