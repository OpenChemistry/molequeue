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

#ifndef AUTHRESPONSEPROCESSOR_H_
#define AUTHRESPONSEPROCESSOR_H_

#include "authenticateresponse.h"
#include "authenticatecont.h"

#include <QtCore/QObject>
#include <QtCore/QList>

namespace MoleQueue
{

class CredentialsDialog;

/**
 * @brief Class use to process a AuthenticateResponse message. Basically walks
 * through the list of prompts provided by the server asking the user for the
 * appropriate responses.
 */
class AuthResponseProcessor : public QObject
{
  Q_OBJECT
public:

  /**
   * @param response The reponse object
   * @param credentialsDialog The dialog that is presented to the user to
   * request responses to each prompt. The class does not assume responsibility
   * for the object.
   */
  AuthResponseProcessor(const AuthenticateResponse &response,
                        CredentialsDialog *credentialsDialog,
                        QObject *parentObject = 0);
  /**
   * Start process the response
   */
  void process();

signals:
  /**
   * Emitted when all the user responses to the prompts have be collected.
   *
   * @param authenticateCont The AuthenticateCont message that can be sent to
   * the UIT server containing the user responses.
   */
  void complete(const AuthenticateCont &authenticateCont);

private slots:
  /**
   * Process the next prompt in the list.
   */
  void nextPrompt();
  /**
   * Set the the user response for the prompt currently being processed
   */
  void processCredentials(const QString &credentials);

private:
  AuthenticateResponse m_authenticateResponse;
  int m_currentIndex;
  CredentialsDialog *m_credentialsDialog;
  /// list contain prompts with user responses filled out.
  QList<Prompt> m_prompts;
};

} /* namespace MoleQueue */

#endif /* AUTHRESPONSEPROCESSOR_H_ */
