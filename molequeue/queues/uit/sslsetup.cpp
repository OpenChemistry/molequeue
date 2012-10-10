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

#include <QtNetwork/QSslSocket>
#include <QtCore/QMutex>
#include <QtCore/QCoreApplication>

namespace MoleQueue
{

bool SslSetup::sslCertsLoaded = false;

SslSetup::SslSetup()
{

}

void SslSetup::init()
{
  static QMutex mutex;

  if(!sslCertsLoaded) {
    QString certDir = QCoreApplication::applicationDirPath() + "/../"
                        + SSL_CERT_DIR;

    QMutexLocker locker(&mutex);

    if(!sslCertsLoaded) {
      bool added = QSslSocket::addDefaultCaCertificates(certDir + "/*",
                                                        QSsl::Pem,
                                                        QRegExp::Wildcard);

      if(!added)
        Logger::logError(QObject::tr("Error adding SSL certificates from %1")
                           .arg(certDir));
      sslCertsLoaded = true;
    }
  }
}

} /* namespace MoleQueue */
