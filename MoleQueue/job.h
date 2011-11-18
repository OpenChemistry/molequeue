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

class Job : public QObject
{
  Q_OBJECT

public:
  explicit Job(const Program *program);
  ~Job();

  void setName(const QString &name);
  QString name() const;
  void setTitle(const QString &title);
  QString title() const;

  const Program* program() const;
  const Queue* queue() const;

private:
  QString m_name;
  QString m_title;
  const Program* m_program;
};

} // end MoleQueue namespace

#endif // JOB_H
