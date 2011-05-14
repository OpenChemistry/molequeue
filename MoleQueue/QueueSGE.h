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

#ifndef QueueSGE_H
#define QueueSGE_H

#include "queue.h"

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
  explicit QueueSGE(QObject *parent = 0);
  ~QueueSGE();

public slots:
  /**
   * Submit a new job to the queue.
   * \param job The Program object to submit to the queue.
   * \return True on successful addition to the queue.
   */
  virtual bool submit(const Program &job);

protected slots:
  /** Job started successfully. */
  virtual void jobStarted(Program *job);

  /** Job completed successfully. */
  virtual void jobFinished(Program *job);

  /** Slot for polling remote jobs that are currently active. */
  virtual void pollRemote();

protected:
  /** Set up some default programs. */
  virtual void setupPrograms();

  /** Set up our SSH connection. */
  virtual void setupProcess();

  /** Submit the job to the remote queue. */
  virtual void submitJob(int index);

  /** Poll the job to see if it is complete. */

  /** Push files to the remote host. */

  /** Retrieve files from the remote host. */

  /** Our SSH connection to the remote host. */
  SshCommand *m_ssh;

  /** A timer for polling the remote host if jobs are active. **/
  QTimer *m_timer;

  /** The interval, in seconds, to poll the remote host. Default is 10 seconds. **/
  int m_interval;

  /** A map of all active remote jobs, associated with their unique remote id. **/
  QMap<QString, Program *> m_remoteJobs;

  /** The local directory used to stage files, and retrieve them. **/
  QString m_localDir;
};

} // End namespace

#endif // QueueSGE_H
