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

#include "kerberoscredentials.h"
#include "authenticator.h"
#include "credentialsdialog.h"
#include "../queueuit.h"


#include "mainwindow.h"
#include "logger.h"

namespace MoleQueue {
namespace Uit {

Authenticator::Authenticator(UitapiService *uit,
                             const QString &kerberosPrinciple,
                             QObject *parentObject,
                             QWidget *dialogParent)
  : QObject(parentObject), m_uit(uit), m_dialogParent(dialogParent),
    m_kerberosPrinciple(kerberosPrinciple), m_credentialsDialog(NULL)
{
  connect(m_uit, SIGNAL(authenticateUserError(const KDSoapMessage&)),
          this, SLOT(authenticateUserError(const KDSoapMessage&)));

}

Authenticator::~Authenticator()
{

}

void Authenticator::authenticate()
{
  showCredentialsDialog(tr("Enter Kerberos credentials for '%1'")
                          .arg(m_kerberosPrinciple),
                        "Password",
                        SLOT(authenticateKerberosCredentials(const QString&)));
}

void Authenticator::authenticateKerberosCredentials(const QString &password)
{
  Uit::KerberosCredentials credentials(m_kerberosPrinciple, password);

  m_credentialsDialog->disconnect(SIGNAL(entered(const QString&)));

  // disconnect to make sure we don't connect twice, is there a better way?
  m_uit->disconnect(SIGNAL(authenticateUserDone(const QString&)));
  connect(m_uit, SIGNAL(authenticateUserDone(const QString&)),
          this, SLOT(authenticateKerberosResponse(const QString&)));


  m_uit->asyncAuthenticateUser(credentials.toXml(),
                               ::MoleQueue::QueueUit::clientId);
}

void Authenticator::authenticateKerberosResponse(const QString &responseXml)
{
  disconnect(m_uit, SIGNAL(authenticateUserDone(const QString&)),
             this, SLOT(authenticateKerberosResponse(const QString&)));

  AuthenticateResponse response = AuthenticateResponse::fromXml(responseXml);
  if (!response.isValid()) {
    QString errorMessage("Server return an invalid authenticate response to " \
                         "kerberos credentials");
    Logger::logError(errorMessage);
    emit authenticationError(errorMessage);
    return;
  }

  // If error message was return display and prompt user again.
  if (response.errorMessage().length() == 0) {
    m_credentialsDialog->setErrorMessage(response.errorMessage());
    m_credentialsDialog->show();
  }
  else {
    m_credentialsDialog->close();

    // disconnect to make sure we don't connect twice, is there a better way?
    m_uit->disconnect(SIGNAL(authenticateUserDone(const QString&)));
    // reconnect authenticateUserDone signal and move to next step
    connect(m_uit, SIGNAL(authenticateUserDone(const QString&)),
            this, SLOT(authenticateResponse(const QString&)));

    authenticateResponse(response);
  }
}

void Authenticator::authenticateResponse(const AuthenticateResponse &response)
{

  m_authSessionId = response.authSessionId();

  if (response.hasPrompts()) {
    // Walk through each prompts getting the credentials from the user.
    AuthResponseProcessor *processor = new AuthResponseProcessor(response,
                                             m_credentialsDialog);

    connect(processor, SIGNAL(complete(const AuthenticateCont&)),
            this, SLOT(authenticateCont(const AuthenticateCont&)));

    processor->process();
  }
  else {
    // If the call was successfully and there are no more prompts
    // then we are authenticated
    if (response.success()) {

      emit authenticationComplete(response.token());

      // We are done ...
      return;
    }
    else {
      // The server has provided a reason for the failure, display to the user
      // and start the process again.
      if (response.errorMessage().length() != 0) {
        m_credentialsDialog->setErrorMessage(response.errorMessage());
        showKerberosCredentialsDialog();
      }
      else {
        QString errorMessage("An error occurred authenticating, server " \
                             "provided no error message.");
        Logger::logError(errorMessage);
        emit authenticationError(errorMessage);
      }
    }
  }
}

void Authenticator::authenticateResponse(const QString &responseXml)
{
  AuthenticateResponse response = AuthenticateResponse::fromXml(responseXml);
  if (!response.isValid()) {
    QString errorMessage = tr("Server return an invalid authenticate response");
    Logger::logError(errorMessage);
    emit authenticationError(errorMessage);
    return;
  }

  authenticateResponse(response);
}

void Authenticator::authenticateCont(const AuthenticateCont &authCont)
{
  AuthResponseProcessor *processor
    = qobject_cast<AuthResponseProcessor*>(sender());

  if (processor) {
    QString response = authCont.toXml();

    m_uit->asyncAuthenticateUser(response, ::MoleQueue::QueueUit::clientId);

    processor->deleteLater();
  }
  else {
    Logger::logError("Unable to get PromptProcessor");
  }
}

void Authenticator::authenticateUserError(const KDSoapMessage& fault)
{
  emit authenticationError(fault.faultAsString());
}


void Authenticator::showKerberosCredentialsDialog()
{
  showCredentialsDialog(tr("Enter Kerberos credentials for '%1'")
                         .arg(m_kerberosPrinciple),
                         "Password",
                         SLOT(authenticateKerberosCredentials(const QString&)));
}

void Authenticator::showCredentialsDialog(const QString banner,
                                          const QString prompt,
                                          const char *enteredSlot)
{

  if (m_credentialsDialog == NULL)
    m_credentialsDialog = new CredentialsDialog(m_dialogParent);

  m_credentialsDialog->setPrompt(prompt);
  m_credentialsDialog->setHostString(banner);

  m_credentialsDialog->disconnect();
  connect(m_credentialsDialog, SIGNAL(entered(const QString&)),
          this, enteredSlot);

  connect(m_credentialsDialog, SIGNAL(canceled()),
          this, SIGNAL(authenticationCanceled()));

  m_credentialsDialog->show();
}

} /* namespace Uit */
} /* namespace MoleQueue */
