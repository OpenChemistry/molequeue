/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2012-2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MOLEQUEUE_CLIENT_H
#define MOLEQUEUE_CLIENT_H

#include "molequeueclientexport.h"

#include <qjsonobject.h>
#include <QtCore/QObject>

#include <QtCore/QHash>

namespace MoleQueue
{

class JsonRpcClient;
class JobObject;

/*!
 * \class Client client.h <molequeue/client/client.h>
 * \brief The Client class is used by clients to submit jobs to a running
 * MoleQueue server.
 * \author Marcus D. Hanwell
 *
 * Provides a simple Qt C++ API to use the MoleQueue JSON-RPC calls to submit
 * and query the state of submitted jobs.
 */

class MOLEQUEUECLIENT_EXPORT Client : public QObject
{
  Q_OBJECT

public:
  explicit Client(QObject *parent_ = 0);
  ~Client();

  /**
   * Query if the client is connected to a server.
   * @return True if connected, false if not.
   */
  bool isConnected() const;

public slots:
  /*!
   * Connect to the server.
   * \param serverName Name of the socket to connect to, the default of
   * "MoleQueue" is usually correct when connecting to the running MoleQueue.
   */
  bool connectToServer(const QString &serverName = "MoleQueue");

  /*!
   * Request the list of queues and programs from the server. The signal
   * queueListReceived() will be emitted once this has been received.
   * @return The local ID of the job submission request.
   */
  int requestQueueList();

  /*!
   * Submit a job to MoleQueue. If the returned local ID is retained the signal
   * for a job submission will provide the MoleQueue ID along with the local ID.
   * @param job The job specification to be submitted to MoleQueue.
   * @return The local ID of the job submission request.
   */
  int submitJob(const JobObject &job);

  /*!
   * Request information about a job. You should supply the MoleQueue ID that
   * was received in response to a job submission.
   * @param moleQueueId The MoleQueue ID for the job.
   * @return The local ID of the job submission request.
   */
  int lookupJob(unsigned int moleQueueId);

  /**
   * Cancel a job that was submitted.
   * @param moleQueueId The MoleQueue ID for the job.
   * @return The local ID of the job submission request.
   */
  int cancelJob(unsigned int moleQueueId);

  /**
   * @brief flush Flush all pending messages to the server.
   * @warning This should not need to be called if used in an event loop, as Qt
   * will start writing to the socket as soon as control returns to the event
   * loop.
   */
  void flush();

signals:
  /*!
   * Emitted when the connection state changes.
   */
  void connectionStateChanged();

  /*!
   * Emitted when the remote queue list is received. This gives a list of lists,
   * the primary key is the queue name, and that contains a list of available
   * programs for each queue.
   * @param queues A JSON object containing the names of the queues and the
   * programs each queue have available.
   */
  void queueListReceived(QJsonObject queues);

  /*!
   * Emitted when the job request response is received.
   * @param localId The local ID the job submission response is in reply to.
   * @param moleQueueId The remote MoleQueue ID for the job submission (can be
   * used to perform further actions on the job).
   */
  void submitJobResponse(int localId, unsigned int moleQueueId);

  /*!
   * Emitted when a job lookup response is received.
   * @param localId The local ID the job submission response is in reply to.
   * @param jobInfo A Json object containing all available job information.
   */
  void lookupJobResponse(int localId, QJsonObject jobInfo);

  /*!
   * Emitted when a job is successfully cancelled.
   */
  void cancelJobResponse(unsigned int moleQueueId);

  /*!
   * Emitted when the job state changes.
   */
  void jobStateChanged(unsigned int moleQueueId, QString oldState,
                       QString newState);

  /*!
   * Emitted when an error response is received.
   */
  void errorReceived(int localId, unsigned int moleQueueId, QString error);

  /*!
   * Emitted when data from the RPC server could not be parsed/used.
   */
  void errorReceived(QString error);

protected slots:
  /*! Parse the response object and emit the appropriate signal(s). */
  void processResult(const QJsonObject &response);

  /*! Parse a notification object and emit the appropriate signal(s). */
  void processNotification(const QJsonObject &notification);

  /*! Parse an error object and emit the appropriate signal(s). */
  void processError(const QJsonObject &notification);

protected:
  enum MessageType {
    Invalid = -1,
    ListQueues,
    SubmitJob,
    CancelJob,
    LookupJob
  };

  JsonRpcClient *m_jsonRpcClient;
  QHash<unsigned int, MessageType> m_requests;
};

} // End namespace MoleQueue

#endif // MOLEQUEUE_CLIENT_H
