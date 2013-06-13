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

#ifndef AUTHENTICATECONT_H_
#define AUTHENTICATECONT_H_

#include "authenticateresponse.h"

namespace MoleQueue {
namespace Uit {

/// @brief class using to model UIT AuthenticateCont message.
class AuthenticateCont
{

public:
  /**
   * @param authSessionId The current UIT auth session ID
   * @param prompts The prompts that the server has requested, including the
   * user responses,
   */
  AuthenticateCont(const QString authSessionId,
                   const QList<Prompt> prompts);

  /**
   * @return the XML representation of this instance to send to the UIT server.
   */
  QString toXml() const;

private:
  QString m_authSessionId;
  QList<Prompt> m_prompts;
};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* AUTHENTICATECONT_H_ */
