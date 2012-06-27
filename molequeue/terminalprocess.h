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

#ifndef TERMINALPROCESS_H
#define TERMINALPROCESS_H

#include <QtCore/QProcess>

namespace MoleQueue {

/**
 * @class Server server.h <molequeue/server.h>
 * @brief Special QProcess derived class, calls setsid on Unix to remove tty,
 * allowing us to give a GUI prompt for SSH etc.
 * @author Marcus D. Hanwell
 */
class TerminalProcess : public QProcess
{
  Q_OBJECT

public:
  explicit TerminalProcess(QObject *parentObject = 0);
  ~TerminalProcess();

protected:
  virtual void setupChildProcess();
};

} // End namespace

#endif // TERMINALPROCESS_H
