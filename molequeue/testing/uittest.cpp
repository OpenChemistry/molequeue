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

#include <QtTest>
#include <QtXmlPatterns/QXmlQuery>
#include <QtCore/QList>
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslConfiguration>

#include "queues/uit/sslsetup.h"

class QueueUitTest : public QObject
{
  Q_OBJECT

private slots:
  void testSslSetup();
};

void QueueUitTest::testSslSetup()
{
  qRegisterMetaType<QList<QSslError> >("QList<QSslError>");

  MoleQueue::SslSetup::init();

  QSslSocket socket;

  // Does our Qt have SSL support?
  QVERIFY(socket.supportsSsl());

  socket.connectToHostEncrypted( "www.uit.hpc.mil", 443 );

  socket.write( "GET / HTTP/1.1\r\n" \
                "Host: www.uit.hpc.mil\r\n" \
                "Connection: Close\r\n\r\n" );

  QSignalSpy spy (&socket, SIGNAL(sslErrors(const QList<QSslError>&)));

  while ( socket.waitForReadyRead() ) {
    qDebug() <<  socket.readAll().data();
  }

  // If SSL certificates are setup correctly we shouldn't get any errors ...
  QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(QueueUitTest)

#include "uittest.moc"
