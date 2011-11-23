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

#ifndef JOB_H
#define JOB_H

#include <QObject>

namespace MoleQueue {

class Queue;
class Program;

/**
 * Class to represent an execution of a Program.
 */
class Job : public QObject
{
  Q_OBJECT

public:
  /** Creates a new job. */
  explicit Job(const Program *program);

  /* Destroys the job object. */
  ~Job();

  /** Set the name of the job to \p name. */
  void setName(const QString &name);

  /** Returns the name of the job. */
  QString name() const;

  /** Sets the title of the job to \p title. */
  void setTitle(const QString &title);

  /** Returns the title for the job. */
  QString title() const;

  /** Returns the program that the job is a type of. */
  const Program* program() const;

  /** Returns the queue that the job is a member of. */
  const Queue* queue() const;

private:
  /** The name of the job. */
  QString m_name;

  /** The title of the job. */
  QString m_title;

  /** The program that the job is a type of. */
  const Program* m_program;
};

} // end MoleQueue namespace

#endif // JOB_H
