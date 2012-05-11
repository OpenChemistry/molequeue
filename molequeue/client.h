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

#include "molequeueglobal.h"

#include <QtCore/QtContainerFwd>

class QDir;

namespace MoleQueue
{
class JobRequest;

/**
 * @class Client client.h <molequeue/client.h>
 * @brief The Client class is used to submit jobs to the MoleQueue application
 * @author David C. Lonie
 *
 * @todo Detail usage
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
   * Create a new JobRequest through this client and return a reference to it.
   * @return The new JobRequest
   */
  JobRequest & newJobRequest();

  /**
   * Retreive the most recent list of Queues and Programs.
   *
   * @return List of key/value pairs; key is queue name, list is of
   * supported programs.
   */
  mqQueueListType queueList() const;

  /// Used for internal lookup structures
  typedef QMap<mqIdType, mqIdType> PacketLookupTable;

signals:

  /**
   * Emitted when the queue list has been updated from the MoleQueue server.
   *
   * @param list List of key/value pairs; key is queue name, list is of
   * supported programs.
   */
  void queueListUpdated(const mqQueueListType &list) const;

  /**
   * Emitted when a job submission reply is received.
   * @param req The job request.
   * @param success Whether the job submission was successful.
   * @param errorMessage String describing the error occurred. Empty if @a
   * success is true.
   */
  void jobSubmitted(const JobRequest & req, bool success,
                    const QString &errorMessage) const;

  /**
   * Emitted when a job cancellation reply is received.
   * @param req The job request.
   * @param success Whether the job cancellation was successful.
  * @param errorMessage String describing the error occurred. Empty if @a
  * success is true.
   */
  void jobCanceled(const JobRequest & req, bool success,
                   const QString &errorMessage) const;

  /**
   * Emitted when a job changes state. The JobState of @a req will already be
   * set to @a newState.
   *
   * @param req The job request.
   * @param oldState The previous state of the job.
   * @param newState The new state of the job.
   */
  void jobStateChanged(const JobRequest &req,
                       JobState oldState, JobState newState);

public slots:

  /**
   * Request a list of Queues and Programs from the server.
   * @sa queueListUpdated()
   * @sa queueList()
   */
  void requestQueueListUpdate();

  /**
   * Submit the job request to the connected server.
   * @param req The JobRequest
   */
  void submitJobRequest(const JobRequest &req);

  /**
   * Cancel a previously submitted job request.
   * @param req The JobRequest
   */
  void cancelJobRequest(const JobRequest &req);

protected slots:

  /**
   * Called when the JsonRpc instance handles a listQueues response.
   * @param list List of supported Queues/Programs
   */
  void queueListReceived(mqIdType, const mqQueueListType &list);

  /**
   * Called when the JsonRpc instance handles a successful submitJob response.
   *
   * @param moleQueueId Unique MoleQueue identifier for job.
   * @param queueJobId Queue-specific job identifier
   * @param workingDir Local working directory for temporary files.
   */
  void successfulSubmissionReceived(mqIdType, mqIdType moleQueueId,
                                    mqIdType queueJobId,
                                    const QDir &workingDir);

  /**
   * Called when the JsonRpc instance handles an unsuccessful submitJob reply.
   *
   * @param errorCode Indication of why the job failed.
   * @param errorMessage Descriptive string detail the failure.
   */
  void failedSubmissionReceived(mqIdType, JobSubmissionErrorCode errorCode,
                                const QString &errorMessage);

  /**
   * Called when the JsonRpc instance handles a job cancellation response.
   *
   * @param moleQueueId Unique MoleQueue identifier for the job.
   */
  void jobCancellationConfirmationReceived(mqIdType,
                                           mqIdType moleQueueId);

  /**
   * Called when the JsonRpc instance handles a job state change notification.
   *
   * @param moleQueueId Unique MoleQueue identifier for the job.
   * @param oldState Previous state of job.
   * @param newState New state of job.
   */
  void jobStateChangeReceived(mqIdType moleQueueId,
                              JobState oldState, JobState newState);

protected:

  /**
   * Get the job request referenced by the indicated client id.
   *
   * @param clientId Client id of JobRequest
   * @return Requested JobRequest
   */
  JobRequest * jobRequestByClientId(mqIdType clientId);

  /**
   * Get the job request referenced by the indicated client id.
   *
   * @param clientId Client id of JobRequest
   * @return Requested JobRequest
   */
  const JobRequest * jobRequestByClientId(mqIdType clientId) const;

  /**
   * Get the job request referenced by the indicated MoleQueue id.
   *
   * @param moleQueueId MoleQueue id of JobRequest
   * @return Requested JobRequest
   */
  JobRequest * jobRequestByMoleQueueId(mqIdType moleQueueId);

  /**
   * Get the job request referenced by the indicated MoleQueue id.
   *
   * @param moleQueueId MoleQueue id of JobRequest
   * @return Requested JobRequest
   */
  const JobRequest * jobRequestByMoleQueueId(mqIdType moleQueueId) const;

  /// List of all owned job requests
  QVector<JobRequest> *m_jobArray;

  /// Map of submitted jobs pending reply. Key is packet id, value is client id.
  PacketLookupTable *m_submittedLUT;

  /// Map of canceled jobs pending reply. Key is packet id, value is client id.
  PacketLookupTable *m_canceledLUT;

  /// Cached list of queues/programs
  mqQueueListType m_queueList;
};

} // end namespace MoleQueue

#endif // CLIENT_H
