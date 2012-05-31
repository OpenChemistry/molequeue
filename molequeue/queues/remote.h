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

#ifndef QUEUEREMOTE_H
#define QUEUEREMOTE_H

#include "../queue.h"

class QTimer;

namespace MoleQueue {

class QueueManager;
class SshCommand;
class TerminalProcess;

/**
 * Remote queue.
 */

class QueueRemote : public Queue
{
  Q_OBJECT
public:
  explicit QueueRemote(QueueManager *parentManager = 0);
  ~QueueRemote();

  QString typeName() const { return "Remote"; }

  QWidget *settingsWidget() const;

public slots:

  virtual bool submitJob(const MoleQueue::Job *job);
};

} // End namespace

#endif // QUEUEREMOTE_H
