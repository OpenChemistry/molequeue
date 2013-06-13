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

#ifndef MOLEQUEUE_ZEROMQCONNECTIONLISTENER_H
#define MOLEQUEUE_ZEROMQCONNECTIONLISTENER_H

#include <molequeue/servercore/connectionlistener.h>
#include "zeromqconnection.h"

#include <zmq.hpp>

#include <QtCore/QTimer>

namespace MoleQueue
{

/// @brief ConnectionListener subclass using ZeroMQ.
class MOLEQUEUEZEROMQ_EXPORT ZeroMqConnectionListener
  : public ConnectionListener
{
  Q_OBJECT
public:
  ZeroMqConnectionListener(QObject *parentObject,
                           const QString &connectionString);

  /**
   * Start the connection listener, start listening for incoming connections.
   */
  void start();

  /**
   * Stop the connection listener.
   *
   * @param force if true, "extreme" measures may be taken to stop the listener.
   */
  void stop(bool force);

  /**
   * Stop the connection listener without forcing it, equivalent to stop(false)
   *
   * @see stop(bool)
   */
  void stop();

  /**
   * @return the "address" the listener will listen on.
   */
  QString connectionString() const;

private:
  QString m_connectionString;

};

} // namespace MoleQueue

#endif // MOLEQUEUE_ZEROMQCONNECTIONLISTENER_H
