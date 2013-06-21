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

#ifndef MOLEQUEUE_DUMMYCONNECTION_H
#define MOLEQUEUE_DUMMYCONNECTION_H

#include <molequeue/servercore/message.h>

#include <molequeue/servercore/connection.h>

class DummyConnection : public MoleQueue::Connection
{
  Q_OBJECT
public:
  explicit DummyConnection(QObject *parent_ = 0);

  void emitPacketReceived(const MoleQueue::Message &message);

  int messageCount()
  {
    return m_messageQueue.size();
  }

  MoleQueue::Message popMessage()
  {
    if (!m_messageQueue.isEmpty())
      return m_messageQueue.takeFirst();
    return MoleQueue::Message();
  }

  // Reimplemented from base class:
  void open();
  void start();
  void close();
  bool isOpen();
  QString connectionString() const;
  bool send(const MoleQueue::PacketType &packet,
            const MoleQueue::EndpointIdType &endpoint);
  void flush();

  QList<MoleQueue::Message> m_messageQueue;

};

#endif // MOLEQUEUE_DUMMYCONNECTION_H
