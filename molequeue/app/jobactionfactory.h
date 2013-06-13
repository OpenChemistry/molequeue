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

/**
 * @class JobActionFactory jobactionfactory.h <molequeue/jobactionfactory.h>
 * @brief Base class for implementing a factory which creates QActions that
 * operate on Job instances.
 * @author David C. Lonie
 */
class JobActionFactory : public QObject
{
  Q_OBJECT
public:

  /// Flags defining properties of the created QActions
  enum Flag {
    /// Actions may be used as a context menu item.
    ContextItem          = 0x1,
  };
  Q_DECLARE_FLAGS(Flags, Flag)

  JobActionFactory();
  JobActionFactory(const JobActionFactory &other);
  ~JobActionFactory();
  JobActionFactory & operator=(const JobActionFactory &other);

  virtual void readSettings(QSettings &settings);
  virtual void writeSettings(QSettings &settings) const;

  /**
   * Set the Server instance. Called in ActionFactoryManager::addFactory().
   */
  void setServer(Server *s) { m_server = s; }

  /** Return the Server instance. */
  Server * server() const { return m_server; }

  /**
   * A name that uniquely identifies this factory.
   */
  virtual QString name() const = 0;

  /** Clear m_jobs and reset m_attemptedJobAdditions */
  virtual void clearJobs();

  /** @return true if the product QActions operate on multiple jobs. */
  virtual bool isMultiJob() const;

  /**
   * Increment m_attemptedJobAdditions and check if the factory's actions are
   * appropriate for the Job @job by calling isValidForJob. If so, @a job is
   * added to m_jobs.
   * @sa isValidForJob
   */
  virtual bool addJobIfValid(const Job &job);

  /**
   * @return true if the factory's actions are appropriate for @a job.
   */
  virtual bool isValidForJob(const Job &job) const = 0;

  /**
   * @return true if this factory's actions should be placed in a submenu. Use
   * menuText() to get the menu name.
   */
  virtual bool useMenu() const;

  /**
   * @return The text to be used for a submenu containing this factory's items.
   * Call useMenu() to see if this is required.
   */
  virtual QString menuText() const;

  /**
   * @return true if addJobIfValid has been called with any appropriate jobs
   * since the last call to clearJobs().
   */
  virtual bool hasValidActions() const;

  /**
   * Create actions that operate on the Job objects in m_jobs. The caller is
   * responsible for managing the lifetime of the actions (passing them to a
   * QMenu or similar is usually sufficient).
   * @sa hasValidActions()
   * @sa addJobIfValid
   * @sa clearJobs
   */
  virtual QList<QAction *> createActions() = 0;

  /**
   * The "usefulness" of the actions produced by this factory, used to order
   * actions in generated menus, etc. Lower value means higher usefulness.
   */
  virtual unsigned int usefulness() const = 0;

  /**
   * @return A set of JobActionFactory::Flags describe the actions produced by
   * this factory.
   */
  virtual Flags flags() const;

  /**
   * @return Set JobActionFactory::Flags describing the actions produced by
   * this factory.
   */
  virtual void setFlags(Flags f);

protected:
  unsigned int m_attemptedJobAdditions;
  bool m_isMultiJob;
  Server *m_server;
  QList<Job> m_jobs;
  Flags m_flags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(JobActionFactory::Flags)

} // end namespace MoleQueue

#endif // JOBCONTEXTACTIONFACTORY_H
