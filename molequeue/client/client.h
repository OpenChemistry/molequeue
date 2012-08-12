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

#ifndef MOLEQUEUE_CLIENT_H
#define MOLEQUEUE_CLIENT_H

#include "molequeueclientexport.h"

#include <QtCore/QObject>

#include <QtCore/QHash>
#include <QtCore/QStringList>

class QLocalSocket;

namespace Json
{
class Value;
}

namespace MoleQueue
{

class Connection;
class Message;
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

  enum MessageType {
    Submission,
    QueueList,
    JobInfo,
    Result,
    Invalid
  };

  bool isConnected() const;

public slots:
  /*!
   * Connect to the server.
   * \param serverName Name of the socket to connect to, the default of
   * "MoleQueue" is usually correct when connecting to the running MoleQueue.
   */
  void connectToServer(const QString &serverName = "MoleQueue");

  /*!
   * Request the list of queues and programs from the server. The signal
   * queueListUpdated() will be emitted once this has been received.
   */
  void requestQueueList();

  /*!
   * Submit a job to MoleQueue.
   */
  void submitJob(const JobObject &job);

  /*!
   * Request information about a job. You should supply the MoleQueue ID that
   * was received in response to a job submission.
   */
  void lookupJob(unsigned int moleQueueId);

protected slots:
  /*!
   * Read incoming packets of data from the server.
   */
  void readPacket(const QByteArray message);

  /*!
   * Read incoming data, interpret JSON stream.
   */
  void readSocket();

signals:
  /*!
   * Emitted when the connection state changes.
   */
  void connectionStateChanged();

  /*!
   * Emitted when the remote queue list is received. This gives a list of lists,
   * the primary key is the queue name, and that contains a list of available
   * programs for each queue.
   */
  void queueListReceived(QHash<QString, QStringList> queues);

  /*!
   * Emitted when the job request response is received.
   */
  void submitJobResponse(int id);

protected:
  unsigned int m_packetCounter;

  QLocalSocket *m_socket;

  QHash<unsigned int, MessageType> m_requests;

  /*! Create a standard empty JSON-RPC 2.0 packet, the method etc is empty. */
  void emptyRequest(Json::Value &request);

  /*! Send the Json request over the transport. */
  void sendRequest(const Json::Value &request);
};

} // End namespace MoleQueue

#endif // MOLEQUEUE_CLIENT_H
