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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "mqconnectionexport.h"
#include "molequeueglobal.h"
#include "message.h"

#include <QtCore/QObject>

namespace MoleQueue
{


/**
 * @class Connection connection.h <molequeue/connection.h>
 * @brief The Connection class is an interface defining the connection using to
 * communicate between MoleQueue processes. Subclasses provide concrete
 * implements for example based on local socket @see LocalSocketConnection
 */
class MQCONNECTION_EXPORT Connection : public QObject
{
  Q_OBJECT
public:

  /**
   * Constructor.
   *
   * @param parentObject parent
   */
  Connection(QObject *parentObject = 0 ) : QObject(parentObject) {};

  /**
   * Open the connection
   */
  virtual void open() = 0;

  /**
   * Start receiving messages on this connection
   */
  virtual void start() = 0;

  /**
   * Send a message on the connection
   */
  virtual void send(MoleQueue::Message msg) = 0;

  /**
   * Close the connection. Once a conneciton is closed if can't reused.
   */
  virtual void close() = 0;

  /*
   * @return true, if the connection is open ( open has been called,
   * false otherwise
   */
  virtual bool isOpen() = 0;

  /**
   * @return the connect string description the endpoint the connection is
   * connected to.
   */
  virtual QString connectionString() const = 0;

signals:
  /**
   * Emitted when a new message has been received on this connection.
   *
   * @param msg The message received.
   */
  void newMessage(const MoleQueue::Message msg);

  /**
   * Emited when the connection is disconnected.
   */
  void disconnected();
};

} // end namespace MoleQueue

#endif // SERVERCONNECTION_H
