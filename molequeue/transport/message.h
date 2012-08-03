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

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "mqconnectionexport.h"
#include "molequeue/molequeueglobal.h"

namespace MoleQueue
{

typedef QByteArray EndpointId;

/// @brief Transport agnostic encapsulation of a single client-server
/// communication.
class MQCONNECTION_EXPORT Message
{
public:
  Message(PacketType data);
  Message(EndpointId to, EndpointId replyTo, PacketType data);
  Message(EndpointId to, PacketType data);

  EndpointId to() const;
  EndpointId replyTo() const;
  PacketType data() const;

private:
  EndpointId m_to;
  EndpointId m_replyTo;
  PacketType m_data;

};

} /* namespace MoleQueue */

#endif /* MESSAGE_H_ */
