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

#ifndef QUEUESGE_H
#define QUEUESGE_H

#include "remote.h"

class QueueSgeTest;

namespace MoleQueue
{

/// @brief QueueRemote subclass for interacting with Sun Grid Engine.
class QueueSge : public QueueRemote
{
  Q_OBJECT
public:
  explicit QueueSge(QueueManager *parentManager = 0);
  ~QueueSge();

  QString typeName() const { return "Sun Grid Engine"; }

  friend class ::QueueSgeTest;

protected:
  virtual bool parseQueueId(const QString &submissionOutput, IdType *queueId);
  virtual QString generateQueueRequestCommand();
  virtual bool parseQueueLine(const QString &queueListOutput, IdType *queueId,
                              JobState *state);

};

} // End namespace

#endif // QueueSGE_H
