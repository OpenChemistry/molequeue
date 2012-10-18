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

#include <QtCore/QObject>

#include "job.h"
#include "molequeueglobal.h"

#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QPointer>
#include <QtCore/QStringList>

class QSettings;

namespace Json
{
class Value;
}

namespace MoleQueue
{
class AbstractQueueSettingsWidget;
class Job;
class Program;
class QueueManager;
class Server;

/**
 * @class Queue queue.h <molequeue/queue.h>
 * @brief Abstract interface for queuing systems.
 * @author Marcus D. Hanwell, David C. Lonie
 *
 * The Queue interface defines interactions with a distributed resource
 * management system, such as job submission and job status updates. Each
 * Queue object manages a set of Program instances, which contain information
 * about the related task of actually running an executable to do work.
 */
class Queue : public QObject
{
  Q_OBJECT
protected:
  /**
   * Protected constructor. Use QueueManager:::addQueue() method to create new
   * Queue objects.
   */
  explicit Queue(const QString &queueName = "Undefined",
                 QueueManager *parentManager = 0);

public:
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
   * @param filePath Path to the .mqq file with the queue settings.
   *
   * This method calls readJsonSettings to perform the actual setting of
   * state information. Subclasses should reimplement that method.
   */
  bool readSettings(const QString &filePath);

  /**
   * Write settings for the queue, done just before closing the server.
   * The settings are written to
   * [server's local working directory]/config/queues/[queuename].mqq
   *
   * This method calls writeJsonSettings to perform the actual collection of
   * state information. Subclasses should reimplement that method.
   */
  bool writeSettings() const;

  /**
   * Populate the passed QSettings object with this queue's configuration.
   * Sensitive data (such as usernames, etc) and mutatable state data (like
   * current jobs) are not written, see writeSettings() if these are needed.
   * @param includePrograms Export this queue's programs as well. Default: true
   *
   * This method calls writeJsonSettings to perform the actual collection of
   * state information. Subclasses should reimplement that method.
   */
  bool exportSettings(const QString &fileName,
                      bool includePrograms = true) const;

  /**
   * Set this Queue's configuration from the passed QSettings object.
   * Sensitive data (such as usernames, etc) and mutatable state data (like
   * current jobs) are not read, see readSettings() if these are needed.
   * @param includePrograms Import any programs contained in the importer.
   * Default: true
   *
   * This method calls readJsonSettings to perform the actual setting of
   * state information. Subclasses should reimplement that method.
   */
  bool importSettings(const QString &fileName, bool includePrograms = true);

  /**
   * @param mqqFile Filename of the mqq (MoleQueue Queue) file.
   * @return Return the type of queue that is stored in the .mqq file.
   */
  static QString queueTypeFromFile(const QString &mqqFile);

  /**
   * @return The name of the persistant state file used to store this Queue's
   * configuration.
   */
  QString stateFileName() const;

  /**
   * @brief writeJsonSettings Write the queue's internal state into a JSON
   * object.
   * @param value Target JSON object.
   * @param exportOnly If true, instance specific information (e.g. currently
   * running jobs, login details, etc) is omitted.
   * @param includePrograms Whether or not to include the Queue's program
   * configurations.
   * @return True on success, false on failure.
   */
  virtual bool writeJsonSettings(Json::Value &value, bool exportOnly,
                                 bool includePrograms) const;

  /**
   * @brief readJsonSettings Initialize the queue's internal state from a JSON
   * object.
   * @param value Source JSON object.
   * @param importOnly If true, instance specific information (e.g. currently
   * running jobs, login details, etc) is ignored.
   * @param includePrograms Whether or not to include the Queue's program
   * configurations.
   * @return True on success, false on failure.
   *
   * @note When reimplementing this method, verify and parse the Json object
   * into temporary variables, then call the base class implementation and only
   * modify the queue if the call returns true.
   */
  virtual bool readJsonSettings(const Json::Value &value, bool importOnly,
                                bool includePrograms);


  /**
   * Returns a widget that can be used to configure the settings for the
   * queue.
   */
  virtual AbstractQueueSettingsWidget* settingsWidget();

  /**
   * Add a new program to the queue. Program names must be unique in each
   * queue, as they are used to specify which program will be used.
   * @note This Queue instance will take ownership and reparent @a newProgram.
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

  /**
   * @param moleQueueId MoleQueue id of Job of interest.
   * @return The number of time the job has failed if it has encountered an
   * error and is being retried. 0 if the job has not encountered an error, or
   * has exceeded three retries.
   */
  int jobFailureCount(IdType moleQueueId) const
  {
    return m_failureTracker.value(moleQueueId, 0);
  }

  /**
   * @brief replaceLaunchScriptKeywords Replace $$keywords$$ in @a launchScript
   * with queue/job specific values.
   * @param launchScript Launch script to complete.
   * @param job Job data to use.
   */
  virtual void replaceLaunchScriptKeywords(QString & launchScript,
                                           const Job &job,
                                           bool addNewline = true);

  /// For queue creation
  friend class MoleQueue::QueueManager;

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

public slots:
  /**
   * Writes input files and submits a new job to the queue.
   * @param job Job to submit.
   * @return True on success, false on failure.
   * @sa jobSubmitted
   */
  virtual bool submitJob(MoleQueue::Job job) = 0;

  /**
   * @brief killJob Stop the job and remove from the queue. Set the JobState to
   * Killed.
   * @param job Job to kill.
   */
  virtual void killJob(MoleQueue::Job job) = 0;

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
  virtual void jobAboutToBeRemoved(const MoleQueue::Job &job);

  /**
   * Delete the local working directory of @a Job.
   */
  void cleanLocalDirectory(const MoleQueue::Job &job);

protected:
  /// Write the input files for @a job to the local working directory.
  bool writeInputFiles(const Job &job);

  /**
   * @brief addJobFailure Call this when a job encounters a problem but will be
   * retried (e.g. a possible networking failure). The failure will be recorded
   * and the return value will indicate whether to retry the job or not. If this
   * function returns true, the job has failed less than 3 times and an attempt
   * should be made to retry. If it returns false, the job has exceeded the
   * maximum number of retries and should be aborted. The failure count will be
   * reset and an error will be logged if the maximum retries are exceeded.
   * @param moleQueueId
   * @return True if the job should be retried, false otherwise.
   * @see jobFailureCount
   * @see clearJobFailures
   */
  bool addJobFailure(IdType moleQueueId);

  /**
   * @brief clearJobFailures Remove all recorded job failures for a job. This
   * does not necessarily mean that the job is successful, but that it is no
   * longer being retried.
   * @param moleQueueId MoleQueue id of job
   * @see addJobFailure
   * @see jobFailureCount
   */
  void clearJobFailures(IdType moleQueueId)
  {
    m_failureTracker.remove(moleQueueId);
  }

  QueueManager *m_queueManager;
  Server *m_server;

  QString m_name;
  QString m_launchTemplate;
  QString m_launchScriptName;
  QMap<QString, Program *> m_programs;
  /// Lookup table for jobs that are using this Queue. Maps JobId to MoleQueueId.
  QMap<IdType, IdType> m_jobs;
  /// Keeps track of the number of times a job has failed (MoleQueueId to
  /// #failures). Once a job fails three times, it will no longer retry.
  QMap<IdType, int> m_failureTracker;

private:
  /// Private helper function
  bool writeJsonSettingsToFile(const QString &filename, bool exportOnly,
                               bool includePrograms) const;
  /// Private helper function
  bool readJsonSettingsFromFile(const QString &filename, bool importOnly,
                                bool includePrograms);
};

} // End namespace

Q_DECLARE_METATYPE(MoleQueue::Queue*)
Q_DECLARE_METATYPE(const MoleQueue::Queue*)

#endif // QUEUE_H
