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

#ifndef SESSIONMANAGER_H_
#define SESSIONMANAGER_H_

#include "authenticator.h"

#include <QtCore/QObject>
#include <QtCore/QMutex>

namespace MoleQueue {
namespace Uit {

class Session;

/**
 * @brief Singleton to manages create and access to UIT sessions. Sessions are key on
 * Kerberos user name and realm.
 */
class SessionManager: public QObject
{
  Q_OBJECT
public:

  /**
   * @return The single instance of the manager.
   */
  static SessionManager * instance();

  /**
   * Lookup a particular UIT session, creating a new session if necessary.
   */
  Session * session(const QString &userName, const QString &realm);

signals:

  /**
   * Emitted when a session token is available for a session ( i.e. the
   * authentication process is complete.
   *
   * @param token The UIT session token.
   */
  void sessionToken(const QString &token);

  /**
   * Emitted when a error occures will create a UIT session or during the
   * authentication process.
   *
   * @param String describing the error that occurred.
   */
  void requestSessionTokenError(const QString &errorString);

private:
  /**
   * @param parentObject The parent object.
   */
  SessionManager(QObject *parentObject);
  SessionManager(const SessionManager&);            // Not implemented.
  SessionManager& operator=(const SessionManager&); // Not implemented.

  UitapiService m_uit;
  /// Mutex to control access to session map.
  QMutex m_sessionsMutex;
  /// map of "username@realm" strings to UIT sessions.
  QMap<QString, Session *> m_sessions;
};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* SESSIONMANAGER_H_ */
