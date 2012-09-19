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
#include "connection.h"
#include "molequeueglobal.h"
#include "message.h"

namespace MoleQueue
{

Message::Message()
  : m_connection(NULL), m_endpoint(), m_data(),
    m_json(Json::nullValue), m_id(), m_type(INVALID_MESSAGE)
{
}

Message::Message(const PacketType &data_)
  : m_connection(NULL), m_endpoint(), m_data(data_),
    m_json(Json::nullValue), m_id(), m_type(INVALID_MESSAGE)
{
  parse();
}

Message::Message(const Json::Value &json_)
  : m_connection(NULL), m_endpoint(), m_data(),
    m_json(json_), m_id(), m_type(INVALID_MESSAGE)
{
  write();
}

Message::Message(Connection *connection_, const EndpointIdType &endpoint_)
  : m_connection(connection_), m_endpoint(endpoint_), m_data(),
    m_json(Json::nullValue), m_id(), m_type(INVALID_MESSAGE)
{
}

Message::Message(Connection *connection_, const EndpointIdType &endpoint_,
                 const PacketType &data_)
  : m_connection(connection_), m_endpoint(endpoint_), m_data(data_),
    m_json(Json::nullValue), m_id(), m_type(INVALID_MESSAGE)
{
  parse();
}

Message::Message(Connection *connection_, const EndpointIdType &endpoint_,
                 const Json::Value &json_)
  : m_connection(connection_), m_endpoint(endpoint_), m_data(),
    m_json(json_), m_id(), m_type(INVALID_MESSAGE)
{
  write();
}

Message::Message(const Message &other)
  : m_connection(other.m_connection),
    m_endpoint(other.m_endpoint),
    m_data(other.m_data),
    m_json(other.m_json),
    m_id(other.m_id),
    m_type(other.m_type)
{
}

Message::Message(const Message &other, const PacketType &data_)
  : m_connection(other.m_connection),
    m_endpoint(other.m_endpoint),
    m_data(data_),
    m_json(),
    m_id(other.m_id),
    m_type(INVALID_MESSAGE)
{
  parse();
}

Message::Message(const Message &other, const Json::Value &json_)
  : m_connection(other.m_connection),
    m_endpoint(other.m_endpoint),
    m_data(),
    m_json(json_),
    m_id(other.m_id),
    m_type(INVALID_MESSAGE)
{
  write();
}

Message &Message::operator=(const Message &other)
{
  m_connection = other.m_connection;
  m_endpoint = other.m_endpoint;
  m_data = other.m_data;
  m_json = other.m_json;
  m_id = other.m_id;
  m_type = other.m_type;
  return *this;
}

void Message::setData(const PacketType &data_)
{
  m_data = data_;
  parse();
}

void Message::setJson(const Json::Value &json_)
{
  m_json = json_;
  write();
}

bool Message::parse()
{
  Json::Reader reader;
  bool b = reader.parse(m_data.constBegin(), m_data.constEnd(), m_json, false);
  if (!b)
    m_json = Json::nullValue;
  return b;
}

void Message::write()
{
  m_data = m_json.toStyledString().c_str();
}

bool Message::send() const
{
  if (!m_connection || !m_connection->isOpen())
    return false;

  m_connection->send(*this);
  return true;
}

} // end namespace MoleQueue
