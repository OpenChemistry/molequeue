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

#include "dummyconnection.h"

#include <molequeue/servercore/message.h>

#include <qjsondocument.h>

DummyConnection::DummyConnection(QObject *parent_) :
  MoleQueue::Connection(parent_)
{
}

void DummyConnection::emitPacketReceived(const MoleQueue::Message &message)
{
  emit packetReceived(MoleQueue::PacketType(message.toJson()),
                      MoleQueue::EndpointIdType());
}

void DummyConnection::open()
{
}

void DummyConnection::start()
{
}

void DummyConnection::close()
{
}

bool DummyConnection::isOpen()
{
  return true;
}

QString DummyConnection::connectionString() const
{
  return "";
}

bool DummyConnection::send(const MoleQueue::PacketType &packet,
                           const MoleQueue::EndpointIdType &endpoint)
{
  MoleQueue::Message message(
        QJsonDocument::fromJson(QByteArray(packet)).object(), this, endpoint);
  message.parse();
  m_messageQueue.append(message);
  return true;
}

void DummyConnection::flush()
{
}
