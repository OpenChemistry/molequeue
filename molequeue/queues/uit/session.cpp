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

#include "session.h"
#include "authenticator.h"

#include <QtCore/QMutexLocker>

namespace MoleQueue {
namespace Uit {

Session::Session(const QString &username, const QString &realm,
                       QObject *parentObject)
  : QObject(parentObject), m_kerberosUserName(username), m_kerberosRealm(realm),
    m_kerberosPrinciple(username + "@" + realm), m_authenticator(NULL)
{

}

QString Session::token()
{
  return m_token;
}

UitapiService *Session::uitService()
{
  return &m_uit;
}

QString Session::kerberosPrinciple()
{
  return m_kerberosPrinciple;
}

void Session::authenticate(QObject *completeReciever,
                              const char *completeSlot,
                              QObject *errorReceiver,
                              const char *errorSlot)
{
  // Take mutex to serialize authentication within a session.
  QMutexLocker locker(&m_authMutex);

  connect(this, SIGNAL(authenticationComplete(const QString&)),
          completeReciever, completeSlot);

  connect(this, SIGNAL(authenticationError(const QString&)),
          errorReceiver, errorSlot);

  // If we aren't currently authenticating create an Athenticator ...
  if (!m_authenticator) {

    m_authenticator = new Authenticator(&m_uit, kerberosPrinciple(), this);

    connect(m_authenticator, SIGNAL(authenticationComplete(const QString&)),
            this, SLOT(authenticationCompleteInternal(const QString&)));
    connect(m_authenticator, SIGNAL(authenticationError(const QString&)),
              this, SLOT(authenticationErrorInternal(const QString&)));
    connect(m_authenticator, SIGNAL(authenticationCancelled()),
                  this, SLOT(authenticationCancelledInternal()));

    m_authenticator->authenticate();
  }
}

void Session::authenticationCompleteInternal(const QString &tok)
{
  QMutexLocker locker(&m_authMutex);

  m_authenticator->deleteLater();
  m_authenticator = NULL;
  m_token = tok;

  emit authenticationComplete(tok);

  disconnect();
}

void Session::authenticationErrorInternal(const QString &errorString)
{
  QMutexLocker locker(&m_authMutex);

  m_authenticator->deleteLater();
  m_authenticator = NULL;

  emit authenticationError(errorString);

  disconnect();
}

void Session::authenticationCancelledInternal()
{
  QMutexLocker locker(&m_authMutex);

  m_authenticator->deleteLater();
  m_authenticator = NULL;

  emit authenticationError("Authentication process was canceled by user.");

  disconnect();
}

} /* namespace Uit */
} /* namespace MoleQueue */
