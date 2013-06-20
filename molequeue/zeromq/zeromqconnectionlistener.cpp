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

#include "zeromqconnectionlistener.h"
#include <molequeue/servercore/message.h>

#include <assert.h>

#include <QtCore/QTimer>

namespace MoleQueue
{

ZeroMqConnectionListener::ZeroMqConnectionListener(QObject *parentObject,
                                                   const QString &address)
  : ConnectionListener(parentObject),
    m_connectionString(address)
{

}

void ZeroMqConnectionListener::start()
{
  zmq::context_t *zeroContext = new zmq::context_t(1);
  zmq::socket_t *zeroSocket = new zmq::socket_t(*zeroContext, ZMQ_ROUTER);

  QByteArray ba = m_connectionString.toLocal8Bit();
  zeroSocket->bind(ba.data());

  ZeroMqConnection *connection = new ZeroMqConnection(this, zeroContext, zeroSocket);

  emit newConnection(connection);
}

void ZeroMqConnectionListener::stop(bool force)
{
  Q_UNUSED(force)
  // Empty
}

void ZeroMqConnectionListener::stop()
{
  // Empty
}

QString ZeroMqConnectionListener::connectionString() const
{
  return m_connectionString;
}

} /* namespace MoleQueue */
