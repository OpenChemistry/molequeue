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

#include <QObject>

#include "molequeueglobal.h"
#include "thirdparty/jsoncpp/json/json-forwards.h"

class AbstractRpcInterfaceTest;

namespace MoleQueue
{
class JsonRpc;
class Connection;

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
   * Sets this AbstractRpcInterface to use the passed connection.
   * @param socket The Connection to use
   */
  virtual void setConnection(Connection *conn);

  /**
   * Interpret a newly received packet.
   *
   * @param packet The packet
   */
  void readPacket(const MoleQueue::PacketType &packet);

  /**
   * Send a response indicating that an invalid packet (unparsable) has been
   * received.
   *
   * @param packetId The packet identifier
   * @param errorDataObject The Json::Value to be used as the error data.
   */
  void replyToInvalidPacket(const Json::Value &packetId,
                            const Json::Value &errorDataObject);

  /**
   * Send a response indicating that an invalid request has been
   * received.
   *
   * @param packetId The packet identifier
   * @param errorDataObject The Json::Value to be used as the error data.
   */
  void replyToInvalidRequest(const Json::Value &packetId,
                             const Json::Value &errorDataObject);

  /**
   * Send a response indicating that an unknown method has been requested.
   *
   * @param packetId The packet identifier
   * @param errorDataObject The Json::Value to be used as the error data.
   */
  void replyToUnrecognizedRequest(const Json::Value &packetId,
                                  const Json::Value &errorDataObject);

  /**
   * Send a response indicating that a request with invalid parameters has been
   * received.
   *
   * @param packetId The packet identifier
   * @param errorDataObject The Json::Value to be used as the error data.
   */
  void replyToinvalidRequestParams(const Json::Value &packetId,
                                   const Json::Value &errorDataObject);

  /**
   * Send a response indicating that an internal error has occurred.
   *
   * @param packetId The packet identifier
   * @param errorDataObject The Json::Value to be used as the error data.
   */
  void replyWithInternalError(const Json::Value &packetId,
                              const Json::Value &errorDataObject);

protected:

  /**
   * @return The next packet id.
   */
  IdType nextPacketId();

  /// The connection used for interprocess communication
  Connection *m_connection;

  /// The internal JsonRpc object
  JsonRpc *m_jsonrpc;

private:
  /// Counter for packet requests
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
