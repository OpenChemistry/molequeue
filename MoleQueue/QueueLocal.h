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

#ifndef QUEUELOCAL_H
#define QUEUELOCAL_H

#include "queue.h"

#include <QtCore/QProcess>

namespace MoleQueue {

/**
 * Queue for jobs to run locally.
 */

class QueueLocal : public Queue
{
  Q_OBJECT
public:
  explicit QueueLocal(QObject *parent = 0);
  ~QueueLocal();

  QString typeName() const { return "Local"; }

  /**
   * Read settings for the queue, done early on at startup.
   */
  virtual void readSettings(const QSettings &settings);

  /**
   * Write settings for the queue, done just before closing the server.
   */
  virtual void writeSettings(QSettings &settings) const;

  /**
   * Returns a widget that can be used to configure the settings for the
   * queue.
   */
  QWidget* settingsWidget() const;

public slots:
  /**
   * Submit a new job to the queue.
   * \param job The Program object to submit to the queue.
   * \return True on successful addition to the queue.
   */
  virtual bool submit(const Program &job);

protected slots:
  /** Job started successfully. */
  void jobStarted();

  /** Job completed successfully. */
  void jobFinished();
  void jobFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void processStateChanged(QProcess::ProcessState newState);

protected:
  /** Set up some default programs. */
  void setupPrograms();

  void runProgram(int jobId);

  /** The number of cores available. */
  int cores() const;

  QProcess *m_process;
  int m_currentJob;
  int m_cores;
};

} // End namespace

#endif // QUEUELOCAL_H
