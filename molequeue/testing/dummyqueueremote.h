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

#ifndef DUMMYQUEUEREMOTE_H
#define DUMMYQUEUEREMOTE_H

#include "queues/remote.h"

#include "dummysshcommand.h"

#include <QtCore/QPointer>

class QueueRemoteTest;

class DummyQueueRemote : public MoleQueue::QueueRemote
{
  Q_OBJECT
public:
  DummyQueueRemote(const QString &queueName,
                   MoleQueue::QueueManager *parentObject);

  ~DummyQueueRemote();

  QString typeName() const { return "Dummy"; }

  DummySshCommand *getDummySshCommand()
  {
    return m_dummySsh.data();
  }

  friend class QueueRemoteTest;

protected:
  MoleQueue::SshConnection *newSshConnection()
  {
    if (!m_dummySsh.isNull()) {
      m_dummySsh->deleteLater();
      m_dummySsh = NULL;
    }

    m_dummySsh = new DummySshCommand();
    m_dummySsh->setHostName(m_hostName);
    m_dummySsh->setUserName(m_userName);
    m_dummySsh->setPortNumber(m_sshPort);

    return m_dummySsh.data();
  }

  bool parseQueueId(const QString &submissionOutput,
                    MoleQueue::IdType *queueId);

  bool parseQueueLine(const QString &queueListOutput,
                      MoleQueue::IdType *queueId, MoleQueue::JobState *state);

  QPointer<DummySshCommand> m_dummySsh;

};

#endif // DUMMYQUEUEREMOTE_H
