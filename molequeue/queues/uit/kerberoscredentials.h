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

#ifndef KERBEROSCREDENTIALS_H_
#define KERBEROSCREDENTIALS_H_

#include <QtCore/QString>

namespace MoleQueue {
namespace Uit {

/**
 * @brief class used to model UIT KerberosCredentials
 */
class KerberosCredentials
{
public:

  /**
   * @param principle The Kerberos username.
   * @param password The Kerberos password.
   */
  KerberosCredentials(const QString &principle, const QString &password);

  /**
   * @return The XML message for this KerberosCredentials instance.
   */
  QString toXml() const;

private:
  QString m_principle;
  QString m_password;
};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* KERBEROSCREDENTIALS_H_ */
