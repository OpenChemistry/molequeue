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

#ifndef UITAUTHENTICATOR_H_
#define UITAUTHENTICATOR_H_

#include "authenticateresponse.h"
#include "authenticatecont.h"
#include "wsdl_uitapi.h"

#include <QtCore/QObject>

namespace MoleQueue {

class CredentialsDialog;

namespace Uit {

/**
 * @brief class used to perform UIT authentication steps.
 */
class Authenticator : public QObject
{
  Q_OBJECT
public:
  Authenticator(UitapiService *uit, const QString &kerberosPrinciple,
                QObject *parentObject = 0, QWidget *dialogParent = 0);
  ~Authenticator();

signals:
  /**
   * Emitted when the authentication is successfully completed.
   *
   * @param token The user token for the session
   */
  void authenticationComplete(const QString &token);

  /**
   * Emitted if an error occurs during authentication.
   */
  void authenticationError(const QString &errorMessage);


  /**
   * Emitted if an user cancels authentication.
   */
  void authenticationCanceled();


public slots:
  /**
   * Start the process of authenticating with the UIT server.
   */
  void authenticate();

private:
  UitapiService *m_uit;
  QWidget *m_dialogParent;
  QString m_authSessionId;
  QString m_kerberosPrinciple;

  /// Dialog uses to enter credentials
  CredentialsDialog *m_credentialsDialog;

  /**
   * display the credentials dialog used to enter kerberos credentials
   */
  void showKerberosCredentialsDialog();

  /**
   *
   *  @param banner The text to display to the user.
   *  @param prompt The prompt to display to the user.
   *  @param enteredSlot The slot to call when the user has entered a response
   *  to the prompt.
   */
  void showCredentialsDialog(const QString banner,
                             const QString prompt,
                             const char *enteredSlot);

private slots:

  /**
   * Send Kerberos credentials to the UIT server. authenticateKerberosResponse
   * will be called with the servers response.
   *
   * @param password The users Kerberos password.
   */
  void authenticateKerberosCredentials(const QString &password);

  /**
   * Called with the servers response to Kerberos authentication message.
   *
   * @param responseXml The XML containing the servers response.
   */
  void authenticateKerberosResponse(const QString &responseXml);

  /**
   * Called with the servers response to an authenticateUser(...) call.
   * Constructs an AuthenticateResponse from the XML and delegates to the
   * overloaded version.
   *
   * @param responseXml The XML from the server.
   *
   */
  void authenticateResponse(const QString &responseXml);

  /**
   * Process a AuthenticateResponse message. Walk through prompts requesting
   * user responses.
   *
   * @param reponse The reponse instance.
   */
  void authenticateResponse(const AuthenticateResponse &response);

  /**
   * Called by the AuthResponseProcessor. Provide the appropriate
   * AuthenticateCont message containing the user responses that can be sent
   * back to the UIT server
   */
  void authenticateCont(const AuthenticateCont &authenticateCont);

  /**
   * Called is an error occurs during the execution of a authenticateUser(...)
   * call.
   *
   * @fault The KDSoap object contains details of the error.
   */
  void authenticateUserError(const KDSoapMessage &fault);

};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* UITAUTHENTICATOR_H_ */
