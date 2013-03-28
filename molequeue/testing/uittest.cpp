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
#include "queues/queueuit.h"
#include "dummyserver.h"
#include "xmlutils.h"
#include "referencestring.h"
#include "queues/uit/jobeventlist.h"
#include "jobmanager.h"

class QueueUitTest : public QObject
{
  Q_OBJECT

private slots:
  void testSslSetup();
  void testJobIdRegex();
  void testHandleQueueUpdate();
};

void QueueUitTest::testSslSetup()
{
  qRegisterMetaType<QList<QSslError> >("QList<QSslError>");

  MoleQueue::Uit::SslSetup::init();

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

void QueueUitTest::testJobIdRegex()
{
  QString submissionOutput = "Job <75899> is submitted to debug queue.";
  QRegExp parser ("^Job <(\\d+)> .*");

  parser.indexIn(submissionOutput);
  QCOMPARE(parser.cap(1), QString("75899"));
}

void QueueUitTest::testHandleQueueUpdate()
{
  DummyServer server;

  MoleQueue::JobManager *jobManager = server.jobManager();
  MoleQueue::Job jobQueuedRemote = jobManager->newJob();
  jobQueuedRemote.setMoleQueueId(100535);
  jobQueuedRemote.setQueueId(100535);

  MoleQueue::Job jobRunningRemote = jobManager->newJob();
  jobRunningRemote.setMoleQueueId(100536);
  jobRunningRemote.setQueueId(100536);


  MoleQueue::QueueUit queue(server.queueManager());
  queue.m_jobs[100535] = 100535;
  queue.m_jobs[100536] = 100536;

  QString m_jobEventXml
    = XmlUtils::stripWhitespace(
        ReferenceString("uit-ref/jobeventlist.xml"));

  MoleQueue::Uit::JobEventList list
    = MoleQueue::Uit::JobEventList::fromXml(m_jobEventXml);

  QVERIFY(jobQueuedRemote.jobState() != MoleQueue::QueuedRemote);
  QVERIFY(jobRunningRemote.jobState() != MoleQueue::RunningRemote);
  queue.handleQueueUpdate(list.jobEvents());

  QVERIFY(jobQueuedRemote.jobState() == MoleQueue::QueuedRemote);
  QVERIFY(jobRunningRemote.jobState() == MoleQueue::RunningRemote);
}

QTEST_MAIN(QueueUitTest)

#include "uittest.moc"
