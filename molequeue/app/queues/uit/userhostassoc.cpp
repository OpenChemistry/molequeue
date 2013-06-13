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

#include "userhostassoc.h"
#include "messagehandler.h"

#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QAbstractMessageHandler>

namespace MoleQueue {
namespace Uit {

UserHostAssoc::UserHostAssoc()
{

}

UserHostAssoc::UserHostAssoc(const UserHostAssoc &other)
  : m_hostID(other.hostId()), m_hostName(other.hostName()),
    m_systemName(other.systemName()), m_description(other.description()),
    m_account(other.account()), m_transportMethod(other.transportMethod())
{

}

UserHostAssoc &UserHostAssoc::operator=(
  const UserHostAssoc &other)
{
  if (this != &other) {
    m_hostID = other.hostId();
    m_hostName = other.hostName();
    m_systemName = other.systemName();
    m_description = other.description();
    m_account = other.account();
    m_transportMethod = other.transportMethod();
  }

  return *this;
}
} /* namespace Uit */
} /* namespace MoleQueue */
