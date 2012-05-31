/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef QUEUESGE_H
#define QUEUESGE_H

#include "../queue.h"

#include <QtCore/QMap>

class QTimer;

namespace MoleQueue {

class TerminalProcess;
class SshCommand;

/**
 * Remote queue.
 */

class QueueSGE : public Queue
{
  Q_OBJECT
public:
  explicit QueueSGE(QObject *parentObject = 0);
  ~QueueSGE();

  QString typeName() const { return "Remote - SGE"; }

public slots:
  /**
   * Submit a new job to the queue.
   * \param job The Program object to submit to the queue.
   * \return True on successful addition to the queue.
   */
  virtual bool submitJob(const Job *job);
};

} // End namespace

#endif // QueueSGE_H
