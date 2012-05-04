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

#ifndef JOBREQUEST_H
#define JOBREQUEST_H

#include "molequeueglobal.h"

class QString;

namespace MoleQueue
{
class MoleQueueClient;
class JobRequestTester;

/**
 * @class JobRequest jobrequest.h <molequeue/jobrequest.h>
 * @brief Container for client-side job request details
 * @author David C. Lonie
 */
class JobRequest
{
public:
  /**
    * Constructor. Can only be called from MoleQueueClient.
    *
    * @param parent MoleQueueClient parent.
    */
  JobRequest(MoleQueueClient *parent);

  /**
    * Constructor. Can only be called from MoleQueueClient.
    *
    * @param parent MoleQueueClient parent.
    */
  JobRequest(const JobRequest &other);

  /// @param newQueue name of the queue.
  void setQueue(const QString &newQueue) {m_queue = newQueue;}

  /// @return Name of queue to use.
  QString queue() {return m_queue;}

  /// @return Name of queue to use.
  const QString & queue() const {return m_queue;}

  /// @param newProgram Name of the program.
  void setProgram(const QString &newProgram) {m_program = newProgram;}

  /// @return Name of program to run.
  QString program() {return m_program;}

  /// @return Name of program to run.
  const QString & program() const {return m_program;}

  /// @param newDesc Description of job
  void setDescription(const QString &newDesc) {m_description = newDesc;}

  /// @return newDesc Description of job
  QString description() {return m_description;}

  /// @return newDesc Description of job
  const QString & description() const {return m_description;}

  /// @param path Path to input file.
  void setInputAsPath(const QString &path) {m_inputAsPath = path;}

  /// @return Path to input file.
  QString inputAsPath() {return m_inputAsPath;}

  /// @return Path to input file.
  const QString & inputAsPath() const {return m_inputAsPath;}

  /// @param input String containing input file contents. Ignored if inputAsPath
  /// is set.
  void setInputAsString(const QString &input) {m_inputAsString = input;}

  /// @return String containing input file contents. Ignored if inputAsPath
  /// is set.
  QString inputAsString() {return m_inputAsString;}

  /// @return String containing input file contents. Ignored if inputAsPath
  /// is set.
  const QString & inputAsString() const {return m_inputAsString;}

  /// @param path String containing a location to copy the output files to after
  /// the job completes. Ignored if empty.
  void setOutputDirectory(const QString &path) {m_outputDirectory = path;}

  /// @return String containing a location to copy the output files to after
  /// the job completes. Ignored if empty.
  QString outputDirectory() {return m_outputDirectory;}

  /// @return String containing a location to copy the output files to after
  /// the job completes. Ignored if empty.
  const QString & outputDirectory() const {return m_outputDirectory;}

  /// @return Temporary working directory where files are stored during job
  /// execution.
  QString localWorkingDirectory() {return m_localWorkingDirectory;}

  /// @return Temporary working directory where files are stored during job
  /// execution.
  const QString & localWorkingDirectory() const
  {
    return m_localWorkingDirectory;
  }

  /// @param clean If true, delete any working files on the remote server.
  /// Default: false.
  void setCleanRemoteFiles(bool clean) {m_cleanRemoteFiles = clean;}

  /// @return If true, delete any working files on the remote server.
  /// Default: false.
  bool cleanRemoteFiles() const {return m_cleanRemoteFiles;}

  /// @param b If true, copies files back from remote server. Default: true
  void setRetrieveOutput(bool b) {m_retrieveOutput = b;}

  /// @return If true, copies files back from remote server. Default: true
  bool retrieveOutput() const {return m_retrieveOutput;}

  /// @param b If true, the local working files are removed after job is
  /// complete. Should be used with setOutputDirectory. Default: false
  void setCleanLocalWorkingDirectory(bool b) {m_cleanLocalWorkingDirectory = b;}

  /// @return If true, the local working files are removed after job is
  /// complete. Should be used with setOutputDirectory. Default: false
  bool cleanLocalWorkingDirectory() const {return m_cleanLocalWorkingDirectory;}

  /// @param b If true, the job will not appear in the queue. Default: false
  void setHideFromQueue(bool b) {m_hideFromQueue = b;}

  /// @return If true, the job will not appear in the queue. Default: false
  bool hideFromQueue() const {return m_hideFromQueue;}

  /// @param b If true, changes in the job state will trigger a popup
  /// notification from the MoleQueue system tray icon. Default: true
  void setPopupOnStateChange(bool b) {m_popupOnStateChange = b;}

  /// @param b If true, changes in the job state will trigger a popup
  /// notification from the MoleQueue system tray icon. Default: true
  bool popupOnStateChange() const {return m_popupOnStateChange;}

  /// @return Internal MoleQueue identifier
  mqIdType molequeueId() const {return m_molequeueId;}

  /// @return Queue Job ID
  mqIdType queueJobId() const {return m_queueJobId;}

  /// Only MoleQueueClients can modify some properties
  friend class MoleQueueClient;

protected:
  /// @param path Temporary working directory where files are stored during job
  /// execution.
  void setLocalWorkingDirectory(const QString &path)
  {
    m_localWorkingDirectory = path;
  }

  /// @param id Internal MoleQueue identifier
  void setMolequeueId(mqIdType id) {m_molequeueId = id;}

  /// @param id Queue Job ID
  void setQueueJobId(mqIdType id) {m_queueJobId = id;}

  /// MoleQueueClient parent
  MoleQueueClient *m_client;
  /// Name of queue to use
  QString m_queue;
  /// Name of program to run
  QString m_program;
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
  bool m_hideFromQueue;
  /// If true, changes in the job state will trigger a popup notification from
  /// the MoleQueue system tray icon. Default: true
  bool m_popupOnStateChange;
  /// Internal MoleQueue identifier
  mqIdType m_molequeueId;
  /// Queue Job ID
  mqIdType m_queueJobId;
};

} // end namespace MoleQueue

#endif // JOBREQUEST_H
