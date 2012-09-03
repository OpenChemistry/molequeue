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

#ifndef AUTHENTICATERESPONSE_H_
#define AUTHENTICATERESPONSE_H_

#include <QtCore/QString>
#include <QtCore/QList>

namespace MoleQueue
{

/**
 * @brief class used to model a UIT prompt.
 */
class Prompt
{

public:
  /**
   * @param id The prompt is provided by UIT.
   * @param prompt The prompt string for example "Password".
   */
  Prompt(int id, const QString prompt);
  Prompt(const Prompt &p);

  /**
   * @return The prompt ID
   */
  int id() const {
    return m_id;
  }

  /**
   * @return The prompt to display to the user for example "Password".
   */
  QString prompt() const {
    return m_prompt;
  }

  /**
   * @param Set the value the user has entered for this prompt.
   */
  void setUserResponse(const QString &response) {
    m_userResponse = response;
  }

  /**
   * @return The value the user entered for this prompt.
   */
  QString userResponse() const {
    return m_userResponse;
  }

private:
  int m_id;
  QString m_prompt;
  QString m_userResponse;
};

/**
 * @brief class used to model UIT AuthenticateResponse
 */
class AuthenticateResponse
{
public:
  AuthenticateResponse();
  AuthenticateResponse(const AuthenticateResponse &response);
  AuthenticateResponse &operator=(const AuthenticateResponse &other);

  /**
   * @return The current UIT auth session id
   */
  QString authSessionId() const;

  /**
   * @return True is the underlying UIT message has user prompts, false
   * otherwise
   */
  bool hasPrompts() const;

  /**
   * @return The list of prompts that need to presented to the user.
   */
  QList<Prompt> prompts() const;

  /**
   * @return true, if the authenticateUser() call was successful, false
   * otherwise.
   */
  bool success() const;

  /**
   * @return Any error message associated with this response.
   */
  QString errorMessage() const;

  /**
   * @return Banner text to be displayed to the user along with the prompts.
   */
  QString banner() const;

  /**
   * @return The session token returned when authentication was successful.
   */
  QString token() const;

  /**
   * @return true is the XML message provided by the server was valid, false
   * otherwise.
   */
  bool isValid() const;

  /**
   * Static method to create a AuthenticateResponse instance from a incoming
   * XML message.
   *
   * @param The XML message.
   */
  static AuthenticateResponse fromXml(const QString &xml);

private:
  QString m_authSessionId;
  bool m_hasPrompts;
  QList<Prompt> m_prompts;
  bool m_success;
  QString m_errorMessage;
  QString m_banner;
  QString m_token;
  bool m_valid;

  /**
   * Set the XML associated with a AuthenticateResponse instance, this method
   * uses XPath the pull out the pertinent parts of the message and init. the
   * appropriate members.
   */
  void setContent(const QString &xml);
};

} /* namespace MoleQueue */

#endif /* AUTHENTICATERESPONSE_H_ */
