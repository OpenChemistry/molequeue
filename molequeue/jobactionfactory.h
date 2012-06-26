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

#ifndef JOBCONTEXTACTIONFACTORY_H
#define JOBCONTEXTACTIONFACTORY_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class QAction;

namespace MoleQueue
{
class Job;
class Server;

class JobActionFactory : public QObject
{
  Q_OBJECT
public:

  enum Flag {
    ContextItem          = 0x1,
    ProgrammableOpenWith = 0x2
  };
  Q_DECLARE_FLAGS(Flags, Flag)

  JobActionFactory();
  JobActionFactory(const JobActionFactory &other);
  virtual ~JobActionFactory();
  JobActionFactory & operator=(const JobActionFactory &other);

  virtual void readSettings(QSettings &settings);
  virtual void writeSettings(QSettings &settings) const;

  void setServer(Server *s) { m_server = s; }
  Server * server() const { return m_server; }

  virtual void clearJobs();

  virtual bool isMultiJob() const;

  virtual bool addJobIfValid(const Job *job);
  virtual bool isValidForJob(const Job *job) const = 0;

  virtual bool hasValidActions() const;
  virtual QList<QAction *> createActions() = 0;

  /// Lower value means higher usefulness.
  virtual unsigned int usefulness() const = 0;

  virtual Flags flags() const;
  virtual void setFlags(Flags f);

protected:
  unsigned int m_attemptedJobAdditions;
  bool m_isMultiJob;
  Server *m_server;
  QList<const Job*> m_jobs;
  Flags m_flags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(JobActionFactory::Flags)

} // end namespace MoleQueue

#endif // JOBCONTEXTACTIONFACTORY_H
