/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef QUEUEOAR_H
#define QUEUEOAR_H

#include "remotessh.h"

class QueueOarTest;

namespace MoleQueue
{

/// @brief QueueRemote subclass for interacting with a OAR managed queue.
class QueueOar : public QueueRemoteSsh
{
  Q_OBJECT
public:
  explicit QueueOar(QueueManager *parentManager = 0);
  ~QueueOar();

  QString typeName() const { return "OAR"; }

  friend class ::QueueOarTest;

protected:
  QString generateQueueRequestCommand();
  bool parseQueueId(const QString &submissionOutput, IdType *queueId);
  bool parseQueueLine(const QString &queueListOutput, IdType *queueId,
                      JobState *state);

};

} // End namespace MoleQueue

#endif // QUEUEOAR_H
