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

#ifndef USERHOSTASSOC_H_
#define USERHOSTASSOC_H_

#include <QtCore/QString>

namespace MoleQueue {
namespace Uit {

/**
 * @brief class used to model UIT UserHostAssoc XML tag.
 */
class UserHostAssoc
{
public:
  UserHostAssoc();
  /**
   * @param other The instance to copy.
   */
  UserHostAssoc(const UserHostAssoc &other);
  /**
   * @param other The instance to assign.
   */
  UserHostAssoc &operator=(const UserHostAssoc &other);

  /**
   * @return The account string.
   */
  QString account() const
  {
    return m_account;
  }

  /**
   * @param acc The account string.
   */
  void setAccount(const QString& acc)
  {
    m_account = acc;
  }

  /**
   * @return The description field.
   */
  QString description() const
  {
    return m_description;
  }

  /**
   * @param des The description field.
   */
  void setDescription(const QString& des)
  {
    m_description = des;
  }

  /**
   * @return host ID for the host associated with the user.
   */
  qint64 hostId() const
  {
    return m_hostID;
  }

  /**
   * @param id The host ID for the host associated with this user.
   */
  void setHostId(qint64 id)
  {
    m_hostID = id;
  }

  /**
   * @return The host name.
   */
  QString hostName() const
  {
    return m_hostName;
  }

  /**
   * @param name The host name.
   */
  void setHostName(const QString& name)
  {
    m_hostName = name;
  }

  /**
   * @return The system name.
   */
  QString systemName() const
  {
    return m_systemName;
  }

  /**
   * @param name The system name.
   */
  void setSystemName(const QString& name)
  {
    m_systemName = name;
  }

  /**
   * @return The transport method.
   */
  QString transportMethod() const
  {
    return m_transportMethod;
  }

  /**
   * @param method The transport method.
   */
  void setTransportMethod(const QString& method)
  {
    m_transportMethod = method;
  }

private:
  qint64 m_hostID;
  QString m_hostName;
  QString m_systemName;
  QString m_description;
  QString m_account;
  QString m_transportMethod;
};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* USERHOSTASSOC_H_ */
