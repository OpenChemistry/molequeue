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

#ifndef SESSION_H_
#define SESSION_H_

#include "wsdl_uitapi.h"

#include <QtCore/QObject>
#include <QtCore/QMutex>

namespace MoleQueue {
namespace Uit {

class Authenticator;

/**
 * @class Session session.h
 * <molequeue/queue/uit/session.h>
 * @brief The Session class encapsulates a UIT authentication token that can
 * be share across multiple requests.
 *
 */
class Session : public QObject
{
  Q_OBJECT
public:
  /**
   * @param username The Kerberos user name.
   * @param realm The Kerbero realm.
   * @param parentObject The parent object.
   */
  Session(const QString &username, const QString &realm,
          QObject *parentObject = 0);

  /**
   * Called to authenticate the session with the UIT server.
   *
   * @param completeReceiver A pointer to the QObject that will receive the
   * complete() signal when the authentication process is complete.
   * @param completeSlot A slot on the completeReceiver class that will be
   * called when the authentication process is complete.
   * @param errorReceiver A pointer to the QObject that will receive error
   * signal during the authentication process if an error occurs.
   */
  void authenticate(QObject *completeReceiver,
                    const char *completeSlot,
                    QObject *errorReceiver,
                    const char *errorSlot);


  QString kerberosPrinciple();
  QString token();
  UitapiService *uitService();

signals:
  void authenticationComplete(const QString &token);
  void authenticationError(const QString &errorString);

private slots:
  void authenticationCompleteInternal(const QString &token);
  void authenticationErrorInternal(const QString &errorMessage);
  void authenticationCancelledInternal();

private:
  QString m_kerberosUserName;
  QString m_kerberosRealm;
  QString m_kerberosPrinciple;
  UitapiService m_uit;
  QString m_token;
  Authenticator *m_authenticator;
  QMutex m_authMutex;
};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* SESSION_H_ */
