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

#ifndef CONNECTIONLISTENER_H_
#define CONNECTIONLISTENER_H_

#include "mqconnectionlistenerexport.h"
#include <molequeue/molequeueglobal.h>
#include "connection.h"

#include <QtCore/QObject>

namespace MoleQueue
{

/**
 * @class ConnectionListener connectionlistener.h <molequeue/connectionlistener.h>
 * @brief The ConnectionListener class is an interface defining a listener waiting
 * for connection to a server. Implementations should emit the @newConnection()
 * signal. Subclasses provide concrete implements for example based on local sockets
 * @see LocalSocketConnectionListener
 */
class MQCONNECTIONLISTENER_EXPORT ConnectionListener : public QObject
{
  Q_OBJECT
  Q_ENUMS(Error)
public:

  /**
   * Constructor.
   *
   * @param parentObject parent
   */
  ConnectionListener(QObject *parentObject = 0 ) : QObject(parentObject) {};

  /**
   * Start the connection listener, start listening for incoming connections.
   */
  virtual void start() = 0;

  /**
   * Stop the connection listener.
   *
   * @param force if true, "extreme" measures may be taken to stop the listener.
   */
  virtual void stop(bool force) = 0;

  /**
   * Stop the connection listener without forcing it, equivalent to stop(false)
   *
   * @see stop(bool)
   */
  virtual void stop() = 0;


  /**
   * @return the "address" the listener will listen on.
   */
  virtual QString connectionString() const = 0;

  /**
   * Defines the errors that will be emitted by @connectionError()
   */
  enum Error {
      AddressInUseError,
      UnknownError = -1
  };

signals:
  /**
   * Emitted when a new connection is received. The new connection is only
   * valid for the lifetime of the connection listener instance that emitted
   * it.
   *
   * @param The new connection.
   */
  void newConnection(MoleQueue::Connection *connection);

  /**
   * Emitted when an error occurs.
   *
   * @param errorCore The error code @see Error
   * @param message The error message provided by the implementation.
   */
  void connectionError(MoleQueue::ConnectionListener::Error errorCode,
                       const QString &message);
};
} // end namespace MoleQueue

Q_DECLARE_METATYPE(MoleQueue::ConnectionListener::Error)

#endif /* CONNECTIONLISTENER_H_ */
