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

#include "testserver.h"

using namespace MoleQueue;

TestServer::TestServer(PacketType *target)
  : QObject(NULL), m_target(target), m_server(new QLocalServer),
    m_socket(NULL)
{
  if (!m_server->listen(getRandomSocketName())) {
    qWarning() << "Cannot start test server:" << m_server->errorString();
    return;
  }

  connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

TestServer::~TestServer()
{
  if (m_socket != NULL)
    m_socket->disconnect();

  delete m_server;
}
