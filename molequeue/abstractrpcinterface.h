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

#ifndef ABSTRACTRPCINTERFACE_H
#define ABSTRACTRPCINTERFACE_H

#include <QtCore/QObject>

#include "molequeueglobal.h"
#include "transport/message.h"

class AbstractRpcInterfaceTest;

namespace Json
{
class Value;
}

namespace MoleQueue
{
class JsonRpc;
class Connection;
class Message;

/**
 * @class AbstractRpcInterface abstractrpcinterface.h <molequeue/abstractrpcinterface.h>
 * @brief Shared functionality between Client and Server
 * @author David C. Lonie
 */
class AbstractRpcInterface : public QObject
{
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parentObject The parent object
   */
  explicit AbstractRpcInterface(QObject *parentObject = NULL);

  /**
   * Destructor.
   */
  virtual ~AbstractRpcInterface();

  friend class ::AbstractRpcInterfaceTest;

protected slots:

  /**
   * Interpret a newly received packet.
   *
   * @param packet The packet
   */
  void readPacket(const MoleQueue::Message msg);

  /**
   * Send a response indicating that an invalid packet (unparsable) has been
   * received.
   *
   * @param packetId The packet identifier
   * @param errorDataObject The Json::Value to be used as the error data.
   */
  void replyToInvalidPacket(MoleQueue::Connection *connection,
                            const MoleQueue::EndpointId replyTo,
                            const Json::Value &packetId,
                            const Json::Value &errorDataObject);

  /**
   * Send a response indicating that an invalid request has been
   * received.
   *
   * @param packetId The packet identifier
   * @param errorDataObject The Json::Value to be used as the error data.
   */
  void replyToInvalidRequest(MoleQueue::Connection *connection,
                             const MoleQueue::EndpointId replyTo,
                             const Json::Value &packetId,
                             const Json::Value &errorDataObject);

  /**
   * Send a response indicating that an unknown method has been requested.
   *
   * @param packetId The packet identifier
   * @param errorDataObject The Json::Value to be used as the error data.
   */
  void replyToUnrecognizedRequest(MoleQueue::Connection *connection,
                                  const MoleQueue::EndpointId replyTo,
                                  const Json::Value &packetId,
                                  const Json::Value &errorDataObject);

  /**
   * Send a response indicating that a request with invalid parameters has been
   * received.
   *
   * @param packetId The packet identifier
   * @param errorDataObject The Json::Value to be used as the error data.
   */
  void replyToinvalidRequestParams(MoleQueue::Connection *connection,
                                   const MoleQueue::EndpointId replyTo,
                                   const Json::Value &packetId,
                                   const Json::Value &errorDataObject);

  /**
   * Send a response indicating that an internal error has occurred.
   *
   * @param packetId The packet identifier
   * @param errorDataObject The Json::Value to be used as the error data.
   */
  void replyWithInternalError(MoleQueue::Connection *connection,
                              const MoleQueue::EndpointId replyTo,
                              const Json::Value &packetId,
                              const Json::Value &errorDataObject);

protected:

  /**
   * @return The next packet id.
   */
  IdType nextPacketId();

  /// The internal JsonRpc object
  JsonRpc *m_jsonrpc;

private:
  /// Counter for packet requests @todo client side only? But what about notifications?
  IdType m_packetCounter;

public:
  /// @param d Enable runtime debugging if true.
  void setDebug(bool d) {m_debug = d;}
  /// @return Whether runtime debugging is enabled.
  bool debug() const {return m_debug;}

protected:
  /// Toggles runtime debugging
  bool m_debug;

};

} // end namespace MoleQueue

#endif // ABSTRACTRPCINTERFACE_H
