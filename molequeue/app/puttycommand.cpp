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

#include "puttycommand.h"

namespace MoleQueue {

PuttyCommand::PuttyCommand(QObject *parentObject) : SshCommand(parentObject,
                                                               "plink", "pscp")
{
}

PuttyCommand::~PuttyCommand()
{
}

QStringList PuttyCommand::sshArgs()
{
  QStringList args;
  if (!m_identityFile.isEmpty())
    args << "-i" << m_identityFile;
  if (m_portNumber >= 0 && m_portNumber != 22)
    args << "-p" << QString::number(m_portNumber);

  return args;
}

QStringList PuttyCommand::scpArgs()
{
  QStringList args;
  if (!m_identityFile.isEmpty())
    args << "-i" << m_identityFile;
  if (m_portNumber >= 0 && m_portNumber != 22)
    args << "-P" << QString::number(m_portNumber);
  return args;
}

} // End namespace
