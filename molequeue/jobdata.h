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

#ifndef JOBDATA_H
#define JOBDATA_H

#include "molequeueglobal.h"

#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QVariantHash>

class ClientTest;
class JobManagerTest;
class JsonRpcTest;
class ServerConnectionTest;

namespace MoleQueue
{
class Client;
class JobManager;
class Server;

/**
 * @class JobData jobdata.h <molequeue/jobdata.h>
 * @brief Internal container for job details.
 * @author David C. Lonie
 *
 * Each JobData instance stores information about a specific job. All JobData
 * objects are owned by a JobManager, which dispenses JobReferenceBase
 * subclasses (Job and JobRequest) that are used to interact with the JobData
 * members.
 *
 */
class JobData
{
public:

  JobData(JobManager *parentManager);
  JobData(const JobData &);

  /// @return The parent JobManager
  JobManager *jobManager() const { return m_jobManager; }

  /// @param newQueue name of the queue.
  void setQueue(const QString &newQueue) { m_queue = newQueue; }

  /// @return Name of queue to use.
  QString queue() const { return m_queue; }

  /// @param newProgram Name of the program.
  void setProgram(const QString &newProgram) { m_program = newProgram; }

  /// @return Name of program to run.
  QString program() const { return m_program; }

  /// @param state Status of job
  void setJobState(JobState state) { m_jobState = state; }

  /// @return Status of job
  JobState jobState() const { return m_jobState; }

  /// @param newDesc Description of job
  void setDescription(const QString &newDesc) { m_description = newDesc; }

  /// @return newDesc Description of job
  QString description() const { return m_description; }

  /// @param path Path to input file.
  void setInputAsPath(const QString &path) { m_inputAsPath = path; }

  /// @return Path to input file.
  QString inputAsPath() const { return m_inputAsPath; }

  /// @param input String containing input file contents. Ignored if inputAsPath
  /// is set.
  void setInputAsString(const QString &input) { m_inputAsString = input; }

  /// @return String containing input file contents. Ignored if inputAsPath
  /// is set.
  QString inputAsString() const { return m_inputAsString; }

  /// @param path String containing a location to copy the output files to after
  /// the job completes. Ignored if empty.
  void setOutputDirectory(const QString &path) { m_outputDirectory = path; }

  /// @return String containing a location to copy the output files to after
  /// the job completes. Ignored if empty.
  QString outputDirectory() const { return m_outputDirectory; }

  /// @param path Temporary working directory where files are stored during job
  /// execution.
  void setLocalWorkingDirectory(const QString &path)
  {
    m_localWorkingDirectory = path;
  }

  /// @return Temporary working directory where files are stored during job
  /// execution.
  QString localWorkingDirectory() const { return m_localWorkingDirectory; }

  /// @param clean If true, delete any working files on the remote server.
  /// Default: false.
  void setCleanRemoteFiles(bool clean) { m_cleanRemoteFiles = clean; }

  /// @return If true, delete any working files on the remote server.
  /// Default: false.
  bool cleanRemoteFiles() const { return m_cleanRemoteFiles; }

  /// @param b If true, copies files back from remote server. Default: true
  void setRetrieveOutput(bool b) { m_retrieveOutput = b; }

  /// @return If true, copies files back from remote server. Default: true
  bool retrieveOutput() const { return m_retrieveOutput; }

  /// @param b If true, the local working files are removed after job is
  /// complete. Should be used with setOutputDirectory. Default: false
  void setCleanLocalWorkingDirectory(bool b)
  {
    m_cleanLocalWorkingDirectory = b;
  }

  /// @return If true, the local working files are removed after job is
  /// complete. Should be used with setOutputDirectory. Default: false
  bool cleanLocalWorkingDirectory() const
  {
    return m_cleanLocalWorkingDirectory;
  }

  /// @param b If true, the job will not appear in the queue. Default: false
  void setHideFromGui(bool b) { m_hideFromGui = b; }

  /// @return If true, the job will not appear in the queue. Default: false
  bool hideFromGui() const { return m_hideFromGui; }

  /// @param b If true, changes in the job state will trigger a popup
  /// notification from the MoleQueue system tray icon. Default: false
  void setPopupOnStateChange(bool b) { m_popupOnStateChange = b; }

  /// @return If true, changes in the job state will trigger a popup
  /// notification from the MoleQueue system tray icon. Default: false
  bool popupOnStateChange() const { return m_popupOnStateChange; }

  /// @param num The total number of processor cores to use (if applicable).
  /// Default: 1
  void setNumberOfCores(int num) { m_numberOfCores = num; }

  /// @return The total number of processor cores to use (if applicable).
  /// Default: 1
  int numberOfCores() const { return m_numberOfCores; }

  /// @param minutes The maximum walltime for this job in minutes. Setting this
  /// to a value <= 0 will use the queue-specific default max walltime. Only
  /// available for remote queues. Default is -1.
  void setMaxWallTime(int minutes) { m_maxWallTime = minutes; }

  /// @return The maximum walltime for this job in minutes. Setting this to a
  /// value <= 0 will use the queue-specific default max walltime. Only
  /// available for remote queues. Default is -1.
  int maxWallTime() const { return m_maxWallTime; }

  /// @param id Internal MoleQueue identifier
  void setMoleQueueId(IdType id) { m_moleQueueId = id; }

  /// @return Internal MoleQueue identifier
  IdType moleQueueId() const { return m_moleQueueId; }

  /// @param id Queue Job ID
  void setQueueId(IdType id) { m_queueId = id; }

  /// @return Queue Job ID
  IdType queueId() const { return m_queueId; }

  /// @return The Job's internal state as a QVariantHash
  QVariantHash hash() const;

  /// Update the Job's internal state from a QVariantHash
  /// @param hash The Job
  void setFromHash(const QVariantHash &state);

protected:
  /// Parent JobManager
  JobManager *m_jobManager;
  /// Name of queue to use
  QString m_queue;
  /// Name of program to run
  QString m_program;
  /// Current state of job
  JobState m_jobState;
  /// Description of job
  QString m_description;
  /// String containing path to input file.
  QString m_inputAsPath;
  /// String containing input file contents. Ignored if m_inputAsPath is set.
  QString m_inputAsString;
  /// String containing a location to copy the output files to after the job
  /// completes. Ignored if empty.
  QString m_outputDirectory;
  /// Temporary working directory where files are stored during job execution.
  QString m_localWorkingDirectory;
  /// If true, delete any working files on the remote server. Default: false.
  bool m_cleanRemoteFiles;
  /// If true, copies files back from remote server. Default: true
  bool m_retrieveOutput;
  /// If true, the local working files are removed after job is complete. Should
  /// be used with setOutputDirectory. Default: false
  bool m_cleanLocalWorkingDirectory;
  /// If true, the job will not appear in the queue. Default: false
  bool m_hideFromGui;
  /// If true, changes in the job state will trigger a popup notification from
  /// the MoleQueue system tray icon. Default: true
  bool m_popupOnStateChange;
  /// The total number of processor cores to use (if applicable).
  /// Default: 1
  int m_numberOfCores;
  /// The maximum walltime for this job in minutes. Setting this
  /// to a value <= 0 will use the queue-specific default max walltime. Only
  /// available for remote queues. Default is -1.
  int m_maxWallTime;
  /// Internal MoleQueue identifier
  IdType m_moleQueueId;
  /// Queue Job ID
  IdType m_queueId;
};

} // end namespace MoleQueue

Q_DECLARE_METATYPE(MoleQueue::JobData*)
Q_DECLARE_METATYPE(const MoleQueue::JobData*)

#endif // JOB_H
