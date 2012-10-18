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

#include "authresponseprocessor.h"
#include "credentialsdialog.h"

namespace MoleQueue
{

AuthResponseProcessor::AuthResponseProcessor(
  const AuthenticateResponse &response, CredentialsDialog *credentialsDialog,
  QObject *parentObject)
  : QObject(parentObject), m_authenticateResponse(response), m_currentIndex(0),
    m_credentialsDialog(credentialsDialog)
{
  m_prompts = response.prompts();

  connect(m_credentialsDialog, SIGNAL(entered(const QString&)),
          this, SLOT(processCredentials(const QString&)));
}

void AuthResponseProcessor::process()
{
  m_credentialsDialog->setHostString(m_authenticateResponse.banner());
  nextPrompt();
}

void AuthResponseProcessor::nextPrompt()
{
  // We still have prompts to present to the user.
  if (m_currentIndex < m_prompts.size()) {
    Prompt p = m_prompts[m_currentIndex];
    m_credentialsDialog->setPrompt(p.prompt());
    m_credentialsDialog->show();
    m_credentialsDialog->raise();
    m_credentialsDialog->activateWindow();
  }
  // We are done so can contruct a AuthenticateCont message
  else {
    m_credentialsDialog->close();
    m_credentialsDialog->disconnect(SIGNAL(entered(const QString&)));

    AuthenticateCont authCont(m_authenticateResponse.authSessionId(),
                              m_prompts);
    emit complete(authCont);
  }
}

void AuthResponseProcessor::processCredentials(const QString &credentials)
{
  m_prompts[m_currentIndex++].setUserResponse(credentials);
  nextPrompt();
}

} /* namespace MoleQueue */
