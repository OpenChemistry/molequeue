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

#ifndef LOCALSOCKETCLIENT_H_
#define LOCALSOCKETCLIENT_H_

#include "mqlocalsocketclientexport.h"
#include "molequeue/client.h"

namespace MoleQueue
{

class MQLOCALSOCKETCLIENT_EXPORT LocalSocketClient: public MoleQueue::Client
{
public:
  explicit LocalSocketClient(QObject *parentObject = 0);

  /**
   * Connect to the server.
   *
   * @param serverName Name of the socket to connect through. Typically
   * "MoleQueue" -- do not change this unless you know what you are doing.
   */
  void connectToServer(const QString &serverName = "MoleQueue");
};

} /* namespace MoleQueue */

#endif /* LOCALSOCKETCLIENT_H_ */
