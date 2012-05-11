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

class QDataStream;
class QLocalSocket;

namespace MoleQueue
{
class JsonRpc;

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
  explicit AbstractRpcInterface(QObject *parentObject = 0);

  /**
   * Destructor.
   */
  virtual ~AbstractRpcInterface();

signals:
  /**
   * Emitted when a complete packet has been read from the socket.
   *
   * @param packet The packet
   */
  void newPacketReady(const PacketType &packet);

public slots:

protected slots:

  /**
   * Read data from the local socket until a complete packet has been obtained.
   */
  void readSocket();

  /**
   * Interpret a newly received packet.
   *
   * @param packet The packet
   */
  void readPacket(const PacketType &packet);

  /**
   * Write a packet to the local socket.
   *
   * @param packet The packet
   */
  void sendPacket(const PacketType &packet);

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

  /**
   * Write a data header containing the packet size and protocol version to the
   * socket.
   *
   * @param packet The packet
   */
  void writePacketHeader(const PacketType &packet);

  /**
   * @return Whether or not the socket header is complete and ready to read.
   */
  bool canReadPacketHeader();

  /**
   * Read the data header containing the packet size and protocol version from
   * the socket.
   *
   * @return The size of incoming packet in bytes.
   */
  qint64 readPacketHeader();

  /// Current version of the packet header
  quint32 m_headerVersion;

  /// Size of the packet header
  const quint32 m_headerSize;

  /// The size of the currently read packet
  qint64 m_currentPacketSize;

  /// The packet currently being read
  PacketType m_currentPacket;

  /// The local socket used for interprocess communication
  QLocalSocket *m_socket;

  /// The data stream used to interface with the local socket
  QDataStream *m_dataStream;

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
