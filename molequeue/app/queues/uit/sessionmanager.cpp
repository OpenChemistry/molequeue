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

#include "sessionmanager.h"
#include "session.h"

#include <QtCore/QCoreApplication>

namespace MoleQueue {
namespace Uit {

SessionManager::SessionManager(QObject *parentObject)
  : QObject(parentObject)
{

}

SessionManager * SessionManager::instance()
{
  static QMutex mutex;
  static SessionManager *uitSessionManager;
  if (!uitSessionManager) {
    mutex.lock();
    if (!uitSessionManager)
    uitSessionManager = new SessionManager(QCoreApplication::instance());
    mutex.unlock();
  }
  return uitSessionManager;
}

Session * SessionManager::session(const QString &userName,
                                  const QString &realm)
{
  QString principle = userName + "@" + realm;
  Session *sess = m_sessions.value(principle, NULL);

  if (!sess) {
    m_sessionsMutex.lock();
    sess = m_sessions.value(principle, NULL);
    if (!sess) {
      sess = new Session(userName, realm, this);
      m_sessions.insert(principle, sess);
    }
    m_sessionsMutex.unlock();
  }

  return sess;
}

} /* namespace Uit */
} /* namespace MoleQueue */
