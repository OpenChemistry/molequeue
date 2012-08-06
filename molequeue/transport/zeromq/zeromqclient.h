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

#ifndef ZEROMQCLIENT_H_
#define ZEROMQCLIENT_H_

#include "mqzeromqclientexport.h"
#include "client.h"

namespace MoleQueue
{

/// @brief Client subclass using ZeroMQ.
class MQZEROMQCLIENT_EXPORT ZeroMqClient : public MoleQueue::Client
{
public:
  explicit ZeroMqClient(QObject *parentObject = 0);

  /**
   * Connect to the server.
   *
   * @param serverName Name of the socket to connect through. Typically
   * "MoleQueue" -- do not change this unless you know what you are doing.
   */
  void connectToServer(const QString &serverName = "MoleQueue");
};

} /* namespace MoleQueue */

#endif /* ZEROMQCLIENT_H_ */
