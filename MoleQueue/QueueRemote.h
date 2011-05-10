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

#include "queue.h"

namespace MoleQueue {

class TerminalProcess;
class SshCommand;

/**
 * Remote queue.
 */

class QueueRemote : public Queue
{
  Q_OBJECT
public:
  explicit QueueRemote(QObject *parent = 0);
  ~QueueRemote();

public slots:
  /**
   * Submit a new job to the queue.
   * \param job The Program object to submit to the queue.
   * \return True on successful addition to the queue.
   */
  bool submit(const Program &job);

protected slots:
  /** Job started successfully. */
  void jobStarted();

  /** Job completed successfully. */
  void jobFinished();

protected:
  /** Set up some default programs. */
  void setupPrograms();

  /** Set up our SSH connection. */
  void setupProcess();

  /** Submit the job to the remote queue. */
  void submitJob(int index);

  /** Poll the job to see if it is complete. */

  /** Push files to the remote host. */

  /** Retrieve files from the remote host. */

  /** Our SSH connection to the remote host. */
  TerminalProcess *m_process;
  SshCommand *m_ssh;
};

} // End namespace

#endif // QUEUEREMOTE_H
