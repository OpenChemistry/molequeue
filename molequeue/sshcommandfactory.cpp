/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "sshcommandfactory.h"
#include "puttycommand.h"
#include "opensshcommand.h"

#include <QtCore/QMutex>
#include <QtCore/QCoreApplication>

namespace MoleQueue
{

namespace
{
static SshCommandFactory *factoryInstance;
}

SshCommandFactory::SshCommandFactory(QObject *parentObject)
  : QObject(parentObject)
{
}

SshCommandFactory *SshCommandFactory::instance()
{
  static QMutex mutex;
  if (!factoryInstance) {
    mutex.lock();
    if (!factoryInstance)
      factoryInstance = new SshCommandFactory(QCoreApplication::instance());
    mutex.unlock();
  }
  return factoryInstance;
}

SshCommand *SshCommandFactory::newSshCommand()
{
#ifdef WIN32
  return newSshCommand(Putty);
#else
  return newSshCommand(OpenSsh);
#endif
}

SshCommand *SshCommandFactory::newSshCommand(SshClient sshClient)
{
    switch(sshClient) {
    case OpenSsh:
      return new OpenSshCommand();
    case Putty:
      return new PuttyCommand();
    default:
      qFatal("Can not create ssh command for: %d", sshClient);
      return NULL;
    }
}

QString SshCommandFactory::defaultSshCommand()
{
#ifdef WIN32
  return QString("plink");
#else
  return QString("ssh");
#endif
}
QString SshCommandFactory::defaultScpCommand()
{
#ifdef WIN32
  return QString("pscp");
#else
  return QString("scp");
#endif
}
} // End MoleQueue namespace
