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

#ifndef USERHOSTASSOCLIST_H_
#define USERHOSTASSOCLIST_H_

#include "userhostassoc.h"

#include <QtCore/QList>

namespace MoleQueue {
namespace Uit {

/**
 * @brief class used to model a UIT UserHostAssocList XML document.
 */
class UserHostAssocList
{
public:
  /**
   * Static method to create a list UserHostAssoc instance from a incoming
   * XML message.
   *
   * @param The XML message.
   * @return The list of UserHostAssoc object
   */
  static UserHostAssocList fromXml(const QString &xml);

  bool isValid() {
    return m_valid;
  }

  /**
   * @return The list of user host associations.
   */
  QList<UserHostAssoc> userHostAssocs() const {
    return m_userHostAssocs;
  }

  /**
   * @return The raw XML used to generate this instance.
   */
  QString xml() const {
    return m_xml;
  }

private:
  bool m_valid;
  QList<UserHostAssoc> m_userHostAssocs;
  QString m_xml;

  UserHostAssocList();

  /**
   * @param xml The XML to parse to populate this instance.
   */
  void setContent(const QString &xml);
};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* USERHOSTASSOCLIST_H_ */
