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

#ifndef QUEUE_H
#define QUEUE_H

#include "object.h"

#include "molequeueglobal.h"

#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QPointer>
#include <QtCore/QStringList>

class QSettings;

namespace MoleQueue
{
class Job;
class Program;
class QueueManager;
class Server;

/**
 * Abstract queue, generally want QueueLocal, or QueueRemote derived classes
 * as they will provide the facilities required. The queue instances
 * themselves refer to the Program classes to actually run jobs with a
 * particular code.
 *
 * Some of the states are skipped for local jobs where there is no separate
 * queue manager such as SGE or PBS. This queue class is a simple FIFO approach
 * and it is assumed that the remote job management system will implement any
 * more sophisticated job management, balancing, etc.
 */

class Queue : public Object
{
  Q_OBJECT
public:
  explicit Queue(const QString &queueName = "Undefined",
                 QueueManager *parentManager = 0);
  ~Queue();

  /// @return The parent Server
  Server *server() { return m_server; }
  /// @return The parent Server
  const Server *server() const { return m_server; }

  /// @return The parent QueueManager
  QueueManager *queueManager() { return m_queueManager; }
  /// @return The parent Server
  const QueueManager *queueManager() const { return m_queueManager; }

  /**
   * Set the name of the queue. This should be unique, and will be used in the
   * GUI to refer to this queue.
   */
  virtual void setName(const QString &newName) { m_name = newName; }

  /** Get the name of the queue. */
  QString name() const { return m_name; }

  /**
   * Returns the type of the queue as a string.
   */
  virtual QString typeName() const { return "Unknown"; }

  /**
   * Read settings for the queue, done early on at startup.
   */
  virtual void readSettings(QSettings &settings);

  /**
   * Write settings for the queue, done just before closing the server.
   */
  virtual void writeSettings(QSettings &settings) const;

  /**
   * Returns a widget that can be used to configure the settings for the
   * queue.
   */
  virtual QWidget* settingsWidget();

  /**
   * Add a new program to the queue. Program names must be unique in each
   * queue, as they are used to specify which program will be used.
   * @param program The program to be added to the queue.
   * @param replace Defaults to false, if true replace any program with the
   * same name in this queue.
   * @return True on success, false on failure.
   */
  bool addProgram(Program *newProgram, bool replace = false);

  /**
   * Attempt to remove a program from the queue. The program name is used
   * as the criteria to decice which object to remove.
   * @param program The program to be removed from the queue.
   * @return True on success, false on failure.
   */
  bool removeProgram(Program *programToRemove);

  /**
   * Attempt to remove a program from the queue. The program name is used
   * as the criteria to decide which object to remove.
   * @param name The name of the program to be removed from the queue.
   * @return True on success, false on failure.
   */
  bool removeProgram(const QString &programName);

  /**
   * Retrieve the program object associated with the supplied name.
   * @param name The name of the program.
   * @return The Program object, a null pointer is returned if the
   * requested program is not in this queue.
   */
  Program* lookupProgram(const QString &programName) const
  {
    return m_programs.value(programName, NULL);
  }

  /**
   * @return A list of program names available through this queue.
   */
  QStringList programNames() const
  {
    return m_programs.keys();
  }

  /**
   * @return A list of the available Program objects.
   */
  QList<Program *> programs() const
  {
    return m_programs.values();
  }

  /**
   * @return The number of programs belonging to this Queue.
   */
  int numPrograms() const
  {
    return m_programs.size();
  }

  /**
   * @return A string containing a template for the launcher script. For remote
   * queues, this will be a batch script for the queuing system, for local
   * queues this will be a shell script (unix) or batch script (windows).
   *
   * It should contain the token "$$programExecution$$", which will be replaced
   * with program-specific launch details.
   */
  virtual QString launchTemplate() const { return m_launchTemplate; }

  /**
   * @return The filename for the launcher script. For remote
   * queues, this will be a batch script for the queuing system, for local
   * queues this will be a shell script (unix) or batch script (windows).
   */
  QString launchScriptName() const { return m_launchScriptName; }

signals:
  /**
   * Emitted when a new program is added to the Queue.
   * @param name Name of the program.
   * @param program Pointer to the newly added Program object.
   */
  void programAdded(const QString &name, MoleQueue::Program *program);

  /**
   * Emitted when a program is removed from the queue.
   * @param name Name of the program
   * @param program Pointer to the removed Program object.
   * @warning The @program pointer should not be dereferenced, as this signal
   * is often associated with program deletion.
   */
  void programRemoved(const QString &name, MoleQueue::Program *program);

  /**
   * Emitted when either a job changes state, or the queue updates. The job
   * may or may not have changed state. The Server's JobManager will notify
   * of job state changes.
   *
   * @param moleQueueId MoleQueue identifier for job.
   * @param state JobState of job.
   */
  void jobStateUpdate(MoleQueue::IdType moleQueueId, MoleQueue::JobState state);

  /**
   * Emitted when a job is assigned a QueueId by the queuing system.
   * @param moleQueueId MoleQueue identifier for job.
   * @param queueId Queuing system's identifier for job.
   */
  void queueIdUpdate(MoleQueue::IdType moleQueueId, MoleQueue::IdType queueId);

public slots:
  /**
   * Writes input files and submits a new job to the queue.
   * @param job Job to submit.
   * @return True on success, false on failure.
   * @sa jobSubmitted
   */
  virtual bool submitJob(const MoleQueue::Job *job) = 0;

  /**
   * Update the launch script template.
   * @param script The new launch template.
   * @sa launchTemplate
   */
  virtual void setLaunchTemplate(const QString &script)
  {
    m_launchTemplate = script;
  }

  /**
   * Update the launch script name.
   * @param scriptName The new launch script name.
   * @sa launchTemplate
   * @sa launchScript
   */
  virtual void setLaunchScriptName(const QString &scriptName)
  {
    m_launchScriptName = scriptName;
  }

protected slots:
  /**
   * Called when the JobManager::jobAboutToBeRemoved signal is emitted to
   * remove any internal references to the job. Subclasses should reimplement
   * if they hold any state about owned jobs.
   */
  virtual void jobAboutToBeRemoved(const MoleQueue::Job *job);

protected:
  /// Write the input files for @a job to the local working directory.
  bool writeInputFiles(const Job *job);

  QueueManager *m_queueManager;
  Server *m_server;

  QString m_name;
  QString m_launchTemplate;
  QString m_launchScriptName;
  QMap<QString, Program *> m_programs;
  /// Lookup table for jobs that are using this Queue. Maps JobId to MoleQueueId.
  QMap<IdType, IdType> m_jobs;
};

} // End namespace

Q_DECLARE_METATYPE(MoleQueue::Queue*)
Q_DECLARE_METATYPE(const MoleQueue::Queue*)

#endif // QUEUE_H
