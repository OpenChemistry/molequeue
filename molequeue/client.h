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

#ifndef CLIENT_H
#define CLIENT_H

#include "abstractrpcinterface.h"

#include "jobrequest.h"
#include "molequeueglobal.h"

#include <QtCore/QtContainerFwd>

class QDir;

class ClientTest;
class ConnectionTest;

namespace MoleQueue
{
class JobManager;

/**
 * @class Client client.h <molequeue/client.h>
 * @brief The Client class is used to submit jobs to the MoleQueue application
 * @author David C. Lonie
 *
 * Programmatic access to the MoleQueue application is provided via the Client
 * class. To use this class, include the molequeue/client.h header and link to
 * the molequeueclient library.
 *
 * An example of simple job submission:
@code
MoleQueue::Client client;
client.connectToServer();
MoleQueue::Job *job = client.newJobRequest();
job->setQueue("QueueName");
job->setProgram("ProgramName");
job->setDescription("Description of job");
job->setInputAsString("[input file contents]");

client.submitJobRequest(job);
@endcode
 *
 * A list of available Queue and Program names can be obtained by calling
 * Client::requestQueueListUpdate(), waiting for the Client::queueListUpdated
 * signal, and taking the output of Client::queueList().
 */
class Client : public AbstractRpcInterface
{
  Q_OBJECT

public:

  /**
   * Constructor.
   *
   * @param parentObject parent
   */
  explicit Client(QObject *parentObject = 0);

  /**
   * Destructor.
   */
  virtual ~Client();

  /**
   * Retreive the most recent list of Queues and Programs.
   *
   * @return List of key/value pairs; key is queue name, list is of
   * supported programs.
   */
  QueueListType queueList() const;

  /**
   * Sets the connection to be used by this client.
   */
  void setConnection(Connection *connecton);

  /// Used for internal lookup structures
  typedef QMap<IdType, JobRequest> PacketLookupTable;

  /// Used for unit testing
  friend class ::ClientTest;
  friend class ::ConnectionTest;

signals:

  /**
   * Emitted when the queue list has been updated from the MoleQueue server.
   *
   * @param list List of key/value pairs; key is queue name, list is of
   * supported programs.
   */
  void queueListUpdated(const MoleQueue::QueueListType &list) const;

  /**
   * Emitted when a job submission reply is received.
   * @param req The job request.
   * @param success Whether the job submission was successful.
   * @param errorMessage String describing the error occurred. Empty if @a
   * success is true.
   */
  void jobSubmitted(const MoleQueue::JobRequest &req, bool success,
                    const QString &errorMessage) const;

  /**
   * Emitted when a job cancellation reply is received.
   * @param req The job request.
   * @param success Whether the job cancellation was successful.
   * @param errorMessage String describing the error occurred. Empty if @a
   * success is true.
   */
  void jobCanceled(const MoleQueue::JobRequest &req, bool success,
                   const QString &errorMessage) const;

  /**
   * Emitted when a job lookup reply is received.
   * @param req The job request. May be invalid if unknown id requested.
   * @param moleQueueId The MoleQueueId from the request.
   * @see lookupJob
   */
  void lookupJobComplete(const MoleQueue::JobRequest &req,
                         MoleQueue::IdType moleQueueId) const;

  /**
   * Emitted when a job changes state. The JobState of @a req will already be
   * set to @a newState.
   *
   * @param req The job request.
   * @param oldState The previous state of the job.
   * @param newState The new state of the job.
   */
  void jobStateChanged(const MoleQueue::JobRequest &req,
                       MoleQueue::JobState oldState,
                       MoleQueue::JobState newState);

public slots:

  /**
   * Connect to the server.
   *
   * @param serverName Name of the socket to connect through. Typically
   * "MoleQueue" -- do not change this unless you know what you are doing.
   */
  virtual void connectToServer(const QString &serverName = "MoleQueue") = 0;

  /**
   * Request a list of Queues and Programs from the server.
   * @sa queueListUpdated()
   * @sa queueList()
   */
  void requestQueueListUpdate();

  /**
   * @return A new Job object to fill with data and submit.
   */
  JobRequest newJobRequest();

  /**
   * Submit the job request to the connected server.
   * @param req The Job
   */
  void submitJobRequest(const JobRequest &req);

  /**
   * Cancel a previously submitted job.
   * @param req The Job
   */
  void cancelJob(const MoleQueue::JobRequest &req);

  /**
   * Request details about a job. If the job with the requested MoleQueue id
   * does not exist in the JobManager, it will be added. Otherwise, the existing
   * job will be updated.
   * @param moleQueueId MoleQueue id of the job.
   * @see lookupJobComplete
   */
  void lookupJob(MoleQueue::IdType moleQueueId);

protected slots:

  /**
   * Called when the JsonRpc instance handles a listQueues response.
   * @param list List of supported Queues/Programs
   */
  void queueListReceived(MoleQueue::IdType,
                         const MoleQueue::QueueListType &list);

  /**
   * Called when the JsonRpc instance handles a successful submitJob response.
   *
   * @param moleQueueId Unique MoleQueue identifier for job.
   * @param queueJobId Queue-specific job identifier
   * @param workingDir Local working directory for temporary files.
   */
  void successfulSubmissionReceived(MoleQueue::IdType,
                                    MoleQueue::IdType moleQueueId,
                                    MoleQueue::IdType queueJobId,
                                    const QDir &workingDir);

  /**
   * Called when the JsonRpc instance handles an unsuccessful submitJob reply.
   *
   * @param errorCode Indication of why the job failed.
   * @param errorMessage Descriptive string detail the failure.
   */
  void failedSubmissionReceived(MoleQueue::IdType,
                                MoleQueue::JobSubmissionErrorCode errorCode,
                                const QString &errorMessage);

  /**
   * Called when the JsonRpc instance handles a job cancellation response.
   *
   * @param moleQueueId Unique MoleQueue identifier for the job.
   */
  void jobCancellationConfirmationReceived(MoleQueue::IdType,
                                           MoleQueue::IdType moleQueueId);

  /**
   * Called when the JsonRpc instance handles a successful lookupJob response.
   *
   * @param hash Hash representing the requested Job's internal state.
   */
  void lookupJobResponseReceived(MoleQueue::IdType,
                                 const QVariantHash & hash);

  /**
   * Called when the JsonRpc instance handles an unsuccessful lookupJob reply.
   * @param moleQueueId Requested MoleQueue id.
   */
  void lookupJobErrorReceived(MoleQueue::IdType, MoleQueue::IdType moleQueueId);

  /**
   * Called when the JsonRpc instance handles a job state change notification.
   *
   * @param moleQueueId Unique MoleQueue identifier for the job.
   * @param oldState Previous state of job.
   * @param newState New state of job.
   */
  void jobStateChangeReceived(MoleQueue::IdType moleQueueId,
                              MoleQueue::JobState oldState,
                              MoleQueue::JobState newState);

protected:

  /// JobManager for this client.
  JobManager *m_jobManager;

  /// Map of submitted jobs pending reply. Key is packet id, value is JobRequest.
  PacketLookupTable *m_submittedLUT;

  /// Map of canceled jobs pending reply. Key is packet id, value is JobRequest.
  PacketLookupTable *m_canceledLUT;

  /// Cached list of queues/programs
  QueueListType m_queueList;

  Connection *m_connection;
};

} // end namespace MoleQueue

#endif // CLIENT_H
