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

#include <json/json.h>

namespace MoleQueue
{
class Connection;

/// Type for Endpoint identifiers
typedef QByteArray EndpointIdType;
/// Type for Message identifiers (JSON-RPC ids)
typedef Json::Value MessageIdType;
/// Type for RPC packets
typedef QByteArray PacketType;

/**
 * @class Message message.h <molequeue/transport/message.h>
 * @brief Message Encapsulation of a single JSON-RPC communication.
 * @author Chris Harris, David C. Lonie
 *
 * The Message class stores the contents and metadata of a JSON-RPC
 * transmission. Connection details, such as the Connection, endpoint, and
 * message id are included to help route replies back to the correct peer.
 * The message contents are stored as both text (data()) and JsonCpp objects
 * (json()). These are kept synced -- setting either will cause the other to
 * be generated internally.
 */

class MQCONNECTION_EXPORT Message
{
public:
  Message();
  explicit Message(const PacketType &data_);
  explicit Message(const Json::Value &json_);
  Message(Connection *connection_, const EndpointIdType &endpoint_);
  Message(Connection *connection_, const EndpointIdType &endpoint_,
          const PacketType &data_);
  Message(Connection *connection_, const EndpointIdType &endpoint_,
          const Json::Value &json_);
  Message(const Message &other);
  /// Use the connection, endpoint, and id of @a other, but the packet in @a
  /// data_.
  Message(const Message &other, const PacketType &data_);
  /// Use the connection, endpoint, and id of @a other, but the packet in @a
  /// json_.
  Message(const Message &other, const Json::Value &json_);
  Message & operator=(const Message &other);

  /// Enumeration of the different JSON-RPC message types.
  enum Type {
    INVALID_MESSAGE = -1,
    REQUEST_MESSAGE,
    RESULT_MESSAGE,
    ERROR_MESSAGE,
    NOTIFICATION_MESSAGE
  };

  /// Copy the connection, endpoint, and message id from @a other into @a this.
  /// @return a reference to @a this Message
  Message &copyConnectionDetails(const Message &other)
  {
    m_connection = other.m_connection;
    m_endpoint = other.m_endpoint;
    m_id = other.m_id;
    return *this;
  }

  /// Connection from which the message originated
  void setConnection(Connection *connection_) { m_connection = connection_; }

  /// Used internally by Connection subclasses.
  void setEndpoint(const EndpointIdType &endpoint_) { m_endpoint = endpoint_; }

  /// The raw message data
  void setData(const PacketType &data_);

  /// The parsed JSON data
  void setJson(const Json::Value &json_);

  /// The ID used in the JSON-RPC call.
  void setId(const MessageIdType &id_) { m_id = id_; }

  /// The type of message.
  void setType(Type type_) { m_type = type_; }

  /// Connection from which the message originated
  Connection *connection() const { return m_connection; }

  /// Used internally by Connection subclasses.
  EndpointIdType endpoint() const { return m_endpoint; }

  /// The raw message data
  PacketType data() const { return m_data; }

  /// The parsed JSON data
  const Json::Value &json() const {return m_json; }

  /// The ID used in the JSON-RPC call.
  MessageIdType id() const { return m_id; }

  /// The type of message.
  Type type() const { return m_type; }

  /// Parse the data() string into the json() member.
  bool parse();

  /// Write the json() object into the data() string.
  void write();

  /// Sends the message. Connection must be set -- this method calls
  /// connection()->send(this)
  /// @return true if the message is sent (e.g. connection is set), false
  /// otherwise.
  bool send() const;

private:
  /// Connection from which the message originated
  Connection *m_connection;
  /// Used internally by Connection subclasses.
  EndpointIdType m_endpoint;
  /// The raw message data
  PacketType m_data;
  /// The parsed JSON data
  Json::Value m_json;
  /// The ID used in the JSON-RPC call.
  MessageIdType m_id;
  /// The type of message.
  Type m_type;

};

} /* namespace MoleQueue */

Q_DECLARE_METATYPE(MoleQueue::Message)
Q_DECLARE_METATYPE(MoleQueue::MessageIdType)

#endif /* MESSAGE_H_ */
