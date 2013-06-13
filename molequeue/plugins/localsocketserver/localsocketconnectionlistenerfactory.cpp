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

#include "localsocketconnectionlistenerfactory.h"

#include <molequeue/servercore/localsocketconnectionlistener.h>

namespace MoleQueue
{

LocalSocketConnectionListenerFactory::LocalSocketConnectionListenerFactory()
{

}

LocalSocketConnectionListenerFactory::~LocalSocketConnectionListenerFactory()
{

}


ConnectionListener *
  LocalSocketConnectionListenerFactory::createConnectionListener(QObject *parentObject,
                                                                 const QString &connectionString)
{
  return new LocalSocketConnectionListener(parentObject, connectionString);
}

} /* namespace MoleQueue */

Q_EXPORT_PLUGIN2(localsocket, MoleQueue::LocalSocketConnectionListenerFactory)
