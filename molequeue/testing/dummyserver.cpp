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

#include "dummyserver.h"

#include <QtCore/QDir>

DummyServer::DummyServer(QObject *parentObject)
  : Server(parentObject, getRandomSocketName())
{
  m_queueManager->deleteLater();
  m_queueManager = new DummyQueueManager (this);
  m_workingDirectoryBase = QDir::tempPath() + "/MoleQueue-dummyServer/";
}

DummyServer::~DummyServer()
{
}
