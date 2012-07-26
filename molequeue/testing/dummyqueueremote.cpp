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

#include "dummyqueueremote.h"

using namespace MoleQueue;

DummyQueueRemote::DummyQueueRemote(const QString &queueName,
                                   QueueManager *parentObject)
  : MoleQueue::QueueRemote(queueName, parentObject),
    m_dummySsh(NULL)
{
  m_launchScriptName = "launcher.dummy";
  m_launchTemplate = "Run job $$moleQueueId$$!!";
}

DummyQueueRemote::~DummyQueueRemote()
{
  if (!m_dummySsh.isNull())
    m_dummySsh->deleteLater();
}

bool DummyQueueRemote::parseQueueId(const QString &submissionOutput,
                                    IdType *queueId)
{
  Q_UNUSED(submissionOutput);
  *queueId = 12;
  return true;
}

bool DummyQueueRemote::parseQueueLine(const QString &queueListOutput,
                                      IdType *queueId, JobState *state)
{
  // Output is "[queueId] [stateAsString]"
  QStringList split = queueListOutput.split(QRegExp("\\s+"));
  if (split.size() < 2)
    return false;

  *queueId = static_cast<IdType>(split.at(0).toULongLong());
  *state = stringToJobState(split.at(1).toUtf8().constData());
  return true;
}
