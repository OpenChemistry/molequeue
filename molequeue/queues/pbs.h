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

#ifndef QUEUEPBS_H
#define QUEUEPBS_H

#include "remote.h"

class QueuePbsTest;

namespace MoleQueue
{

/// @brief QueueRemote subclass for interacting with a PBS/Torque queue.
class QueuePbs : public QueueRemote
{
  Q_OBJECT
public:
  explicit QueuePbs(QueueManager *parentManager = 0);
  ~QueuePbs();

  QString typeName() const { return "PBS/Torque"; }

  friend class ::QueuePbsTest;

protected:
  virtual bool parseQueueId(const QString &submissionOutput, IdType *queueId);
  virtual bool parseQueueLine(const QString &queueListOutput, IdType *queueId,
                              JobState *state);

};

} // End namespace

#endif // QUEUEPBS_H
