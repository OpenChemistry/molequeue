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

#include "userhostassoclist.h"
#include "messagehandler.h"

#include <QtCore/QStringList>
#include <QtXmlPatterns/QXmlQuery>

namespace MoleQueue {
namespace Uit {

UserHostAssocList::UserHostAssocList()
  : m_valid(false)
{
  // TODO Auto-generated constructor stub

}

void UserHostAssocList::setContent(const QString &content)
{
  m_xml = content;
  m_valid = true;

  MessageHandler handler;
  QXmlQuery query;
  query.setMessageHandler(&handler);
  m_valid = query.setFocus(m_xml);

  if (!m_valid)
    return;

  // First get the list of hostIDs
  query.setQuery("/list/PublicHostPlusUser/hostID/string()");
  QStringList hostIDs;
  m_valid = query.evaluateTo(&hostIDs);

  if (!m_valid)
    return;

  QList<UserHostAssoc> assocs;

  foreach (const QString &id, hostIDs) {
    UserHostAssoc userHostAssoc;
    userHostAssoc.setHostId(id.toULongLong());
    query.bindVariable("id", QVariant(id));
    // Get the account
    QString user;
    query.setQuery("/list/PublicHostPlusUser[hostID=$id]/account/string()");
    m_valid = query.evaluateTo(&user);

    if (!m_valid)
        return;

    userHostAssoc.setAccount(user.trimmed());

    // Get the systemName
    QString system;
    query.setQuery("/list/PublicHostPlusUser[hostID=$id]/systemName/string()");
    m_valid = query.evaluateTo(&system);

    if (!m_valid)
        return;

    userHostAssoc.setSystemName(system.trimmed());

    // Get the transportMethod
    QString transport;
    query.setQuery(
      "/list/PublicHostPlusUser[hostID=$id]/transportMethod/string()");
    m_valid = query.evaluateTo(&transport);

    if (!m_valid)
        return;

    userHostAssoc.setTransportMethod(transport.trimmed());

    // Get the description
    QString des;
    query.setQuery("/list/PublicHostPlusUser[hostID=$id]/description/string()");
    m_valid = query.evaluateTo(&des);

    if (!m_valid)
        return;

    userHostAssoc.setDescription(des.trimmed());

    // Get the hostName
    QString host;
    query.setQuery("/list/PublicHostPlusUser[hostID=$id]/hostName/string()");
    m_valid = query.evaluateTo(&host);

    if (!m_valid)
        return;

    userHostAssoc.setHostName(host.trimmed());

    assocs.append(userHostAssoc);
  }

  m_userHostAssocs = assocs;
}

UserHostAssocList UserHostAssocList::fromXml(const QString &xml)
{
  UserHostAssocList list;
  list.setContent(xml);

  return list;
}

} /* namespace Uit */
} /* namespace MoleQueue */
