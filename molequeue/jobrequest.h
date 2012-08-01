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

#ifndef MOLEQUEUE_JOBREQUEST_H
#define MOLEQUEUE_JOBREQUEST_H

#include "jobreferencebase.h"

namespace MoleQueue
{

/**
 * @class JobRequest jobrequest.h <molequeue/jobrequest.h>
 * @brief Client-side interface to JobData properties.
 * @author David C. Lonie
 *
 * The JobRequest class provides a lightweight interface to a specific instance
 * of JobData. Since JobData contains dynamic information that changes during
 * its lifetime, the JobRequest interface forwards requests to a JobData
 * instance, which ensures that all references to job information are synced
 * throughout the application.
 *
 * The JobRequest interface differs from the closely related Job class by
 * providing a restricted set of operations suitable for use by clients. For
 * instance, the MoleQueue id of a job cannot be changed through the JobRequest
 * interface, as this should only be modified internally by MoleQueue.
 *
 * The JobReferenceBase class holds and validates a pointer to a JobData
 * instance and will detect when the associated JobData object is removed from
 * the JobManager (such as when the user deletes a job from the job table). To
 * check the validity of the JobData pointer, use JobRequest::isValid(), defined
 * in JobReferenceBase.
 */
class JobRequest : public JobReferenceBase
{
public:
  /// Construct a new JobRequest object with the specified JobData
  explicit JobRequest(JobData *jobdata = NULL);

  /// Construct a new JobRequest object for the job with the MoleQueueId
  /// in the indicated JobManager
  JobRequest(JobManager *jobManager, IdType mQId);

  /// Construct a new JobRequest object with the same JobData as @a other.
  JobRequest(const JobReferenceBase &other);

  ~JobRequest();

  /// @return The JobData's internal state as a QVariantHash
  QVariantHash hash() const;

  /// @param newQueue name of the queue.
  void setQueue(const QString &newQueue);

  /// @return Name of queue to use.
  QString queue() const;

  /// @param newProgram Name of the program.
  void setProgram(const QString &newProgram);

  /// @return Name of program to run.
  QString program() const;

  /// @return Status of job
  JobState jobState() const;

  /// @param newDesc Description of job
  void setDescription(const QString &newDesc);

  /// @return newDesc Description of job
  QString description() const;

  /// @param path Path to input file.
  void setInputAsPath(const QString &path);

  /// @return Path to input file.
  QString inputAsPath() const;

  /// @param input String containing input file contents. Ignored if inputAsPath
  /// is set.
  void setInputAsString(const QString &input);

  /// @return String containing input file contents. Ignored if inputAsPath
  /// is set.
  QString inputAsString() const;

  /**
   * Set the output directory for the job.
   * If empty, the Server will set it to the temporary working directory once
   * the job is accepted. Otherwise, the output files will be copied to the
   * specified location when the job completes.
   */
  void setOutputDirectory(const QString &path);

  /// @return String containing a location to copy the output files to after
  /// the job completes. Ignored if empty.
  QString outputDirectory() const;

  /// @return Temporary working directory where files are stored during job
  /// execution.
  QString localWorkingDirectory() const;

  /// @param clean If true, delete any working files on the remote server.
  /// Default: false.
  void setCleanRemoteFiles(bool clean);

  /// @return If true, delete any working files on the remote server.
  /// Default: false.
  bool cleanRemoteFiles() const;

  /// @param b If true, copies files back from remote server. Default: true
  void setRetrieveOutput(bool b);

  /// @return If true, copies files back from remote server. Default: true
  bool retrieveOutput() const;

  /// @param b If true, the local working files are removed after job is
  /// complete. Should be used with setOutputDirectory. Default: false
  void setCleanLocalWorkingDirectory(bool b);

  /// @return If true, the local working files are removed after job is
  /// complete. Should be used with setOutputDirectory. Default: false
  bool cleanLocalWorkingDirectory() const;

  /// @param b If true, the job will not appear in the MoleQueue user interface
  /// by default. Useful for automated batch jobs.
  void setHideFromGui(bool b);

  /// @return If true, the job will not appear in the queue. Default: false
  bool hideFromGui() const;

  /// @param b If true, changes in the job state will trigger a popup
  /// notification from the MoleQueue system tray icon. Default: true
  void setPopupOnStateChange(bool b);

  /// @return If true, changes in the job state will trigger a popup
  /// notification from the MoleQueue system tray icon. Default: true
  bool popupOnStateChange() const;

  /// @param num The total number of processors to use (if applicable).
  /// Default: 1
  void setNumberOfProcessors(int num);

  /// @return The total number of processors to use (if applicable).
  /// Default: 1
  int numberOfProcessors() const;

  /// @param minutes The maximum walltime for this job in minutes. Default is
  /// 24x60=1440, e.g. 1 day.
  void setMaxWallTime(int minutes);

  /// @return The maximum walltime for this job in minutes. Default is
  //// 24x60=1440, e.g. 1 day.
  int maxWallTime() const;

  /// @return Internal MoleQueue identifier
  IdType moleQueueId() const;

  /// @return Queue Job ID
  IdType queueId() const;
};

} // namespace MoleQueue

Q_DECLARE_METATYPE(MoleQueue::JobRequest)

#endif // MOLEQUEUE_JOBREQUEST_H
