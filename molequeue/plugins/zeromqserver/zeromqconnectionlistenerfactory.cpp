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

#include "zeromqconnectionlistenerfactory.h"

#include <molequeue/zeromq/zeromqconnectionlistener.h>

#include <QtCore/QDir>

namespace MoleQueue
{

ZeroMqConnectionListenerFactory::ZeroMqConnectionListenerFactory()
{

}

ConnectionListener *ZeroMqConnectionListenerFactory::createConnectionListener(QObject *parentObject,
                                                                              const QString &connectionString)
{
  QString connectionPath = QDir::temp().path() + "/" +
                           ZeroMqConnection::zeroMqPrefix + "_" +
                           connectionString;
  return new ZeroMqConnectionListener(parentObject, "ipc://" + connectionPath);
}

} /* namespace MoleQueue */

Q_EXPORT_PLUGIN2(zeromq, MoleQueue::ZeroMqConnectionListenerFactory)
