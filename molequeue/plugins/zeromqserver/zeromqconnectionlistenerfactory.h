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

#ifndef MOLEQUEUE_ZEROMQCONNECTIONLISTENERFACTORY_H
#define MOLEQUEUE_ZEROMQCONNECTIONLISTENERFACTORY_H

#include <molequeue/servercore/connectionlistenerfactory.h>

namespace MoleQueue
{

/// @brief A ConnectionListenerFactory subclass using ZeroMQ.
class ZeroMqConnectionListenerFactory: public QObject,
                                       public MoleQueue::ConnectionListenerFactory
{
  Q_OBJECT
  Q_INTERFACES(MoleQueue::ConnectionListenerFactory)
public:
  ZeroMqConnectionListenerFactory();
  ConnectionListener *createConnectionListener(QObject *parentObject = 0,
                                               const QString &connectionString = "MoleQueue");
};

} // namespace MoleQueue

#endif // MOLEQUEUE_ZEROMQCONNECTIONLISTENERFACTORY_H
