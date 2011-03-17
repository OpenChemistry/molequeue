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

#ifndef QUEUE_H
#define QUEUE_H

#include <QtCore/QObject>

#include <QtCore/QList>

#include "program.h"

namespace MoleQueue {

/**
 * Abstract queue, generally want QueueLocal, or QueueRemote derived classes
 * as they will provide the facilities required. The queue instances
 * themselves refer to the Program classes to actually run jobs with a
 * particular code.
 *
 * Some of the states are skipped for local jobs where there is no separate
 * queue manager such as SGE or PBS.
 */

class Queue : public QObject
{
  Q_OBJECT
public:
  explicit Queue(QObject *parent = 0);
  ~Queue();

signals:

public slots:
  /**
   * Submit a new job to the queue.
   * \param job The Program object to submit to the queue.
   * \return True on successful addition to the queue.
   */
  bool submit(const Program &job);

protected:
  QList<Program> m_jobs;

};

} // End namespace

#endif // QUEUE_H
