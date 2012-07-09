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

#ifndef CONNECTIONLISTENERFACTORY_H_
#define CONNECTIONLISTENERFACTORY_H_

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtPlugin>

namespace MoleQueue
{

class ConnectionListener;

class ConnectionListenerFactory
{

public:
  virtual ~ConnectionListenerFactory() {};
  virtual ConnectionListener *createConnectionListener(QObject *parentObject,
                                                       QString connectionString) = 0;
};

} /* namespace MoleQueue */

Q_DECLARE_INTERFACE(MoleQueue::ConnectionListenerFactory,
                    "org.openchemistry.molequeue.ConnectionListenerFactory/1.0")

#endif /* CONNECTIONLISTENERFACTORY_H_ */
