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

#ifndef TESTSERVER_H
#define TESTSERVER_H

#include <QtCore/QObject>

#include "molequeueglobal.h"
#include "transport/message.h"

#include <QtGlobal>

#include <QtCore/QDateTime>
#include <QtCore/QThread>
#include <QtCore/QTimer>

#include <QtGui/QApplication>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

class TestServer : public QObject
{
  Q_OBJECT
  MoleQueue::PacketType *m_target;
  QLocalServer *m_server;
  QLocalSocket *m_socket;
public:
  TestServer(MoleQueue::PacketType *target);

  ~TestServer();

  void sendPacket(const MoleQueue::PacketType &packet)
  {
    QDataStream out (m_socket);
    out.setVersion(QDataStream::Qt_4_7);

    out << packet;
    m_socket->flush();
  }

  bool waitForPacket(int timeout_ms = 5000)
  {
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(timeout_ms);
    while (timer.isActive() && m_target->isEmpty()) {
      qApp->processEvents(QEventLoop::AllEvents, 500);
    }
    return !m_target->isEmpty();
  }

  QString socketName() const {return m_server->serverName();}

  static QString getRandomSocketName()
  {
    // Generate a time, process, and thread independent random value.
    quint32 threadPtr = static_cast<quint32>(
          reinterpret_cast<qptrdiff>(QThread::currentThread()));
    quint32 procId = static_cast<quint32>(qApp->applicationPid());
    quint32 msecs = static_cast<quint32>(
          QDateTime::currentDateTime().toMSecsSinceEpoch());
    unsigned int seed = static_cast<unsigned int>(
          (threadPtr ^ procId) ^ ((msecs << 16) ^ msecs));
    qDebug() << "Seed:" << seed;
    qsrand(seed);
    int randVal = qrand();

    return QString("MoleQueue-testing-%1").arg(QString::number(randVal));
  }

private slots:
  void newConnection()
  {
    m_socket = m_server->nextPendingConnection();
    connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
  }

  void readyRead()
  {
//  qDebug() << "Test server received" << m_socket->bytesAvailable() << "bytes.";
    QDataStream in (m_socket);
    in.setVersion(QDataStream::Qt_4_7);

    MoleQueue::PacketType packet;

    in >> *m_target;
  }
};

#endif // TESTSERVER_H
