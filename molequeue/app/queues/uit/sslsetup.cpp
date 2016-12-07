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

#include "sslsetup.h"
#include "logger.h"
#include "molequeueconfig.h"

#include <QtNetwork/QSslSocket>
#include <QtCore/QMutex>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>


#include <QtNetwork/QSslConfiguration>

namespace MoleQueue {
namespace Uit {

bool SslSetup::sslCertsLoaded = false;

SslSetup::SslSetup()
{

}

void SslSetup::init()
{
  static QMutex mutex;

  if (!sslCertsLoaded) {
    QStringList certDirs;
    certDirs << QCoreApplication::applicationDirPath() + "/../"
                 + MoleQueue_SSL_CERT_DIR;
    // for super build
    certDirs << QCoreApplication::applicationDirPath() + "/../molequeue/"
                 + MoleQueue_SSL_CERT_DIR;

    QMutexLocker locker(&mutex);

    QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
    sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(sslConf);

    if (!sslCertsLoaded) {
      foreach(const QString &dir, certDirs) {
        if (QDir(dir).exists()) {
          bool added = QSslSocket::addDefaultCaCertificates(dir + "/*",
                                                            QSsl::Pem,
                                                            QRegExp::Wildcard);

          if (!added) {
            Logger::logError(QObject::tr(
                               "Error adding SSL certificates from %1")
                                 .arg(dir));
          }
        }
      }
      sslCertsLoaded = true;
    }
  }
}

} /* namespace Uit */
} /* namespace MoleQueue */
