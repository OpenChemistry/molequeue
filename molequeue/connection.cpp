/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "connection.h"

#include <QtCore/QDataStream>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QDebug>

namespace MoleQueue {

Connection::Connection(QLocalSocket *socket, QObject *newParent)
  : QObject(newParent), m_socket(socket), m_blockSize(0), m_state(IDLE)
{
  if (m_socket) {
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(newDataReady()));
    sendMessage("Hello from the server...");
  }
}

Connection::~Connection()
{

}

void Connection::newDataReady()
{
  QDataStream in(m_socket);
  in.setVersion(QDataStream::Qt_4_7);

  if (m_blockSize == 0) {
    if (m_socket->bytesAvailable() < static_cast<int>(sizeof(quint16)))
      return;
    in >> m_blockSize;
  }
  if (in.atEnd())
    return;

  // If idle, we expect to receive a command as a QString
  switch (m_state) {
  case IDLE: {
    QString command;
    in >> command;
    qDebug() << "IDLE:" << command;
    if (command == "Submit") {
      m_state = INPUT_FILE;
      sendMessage("Submit: ready");
    }
    m_blockSize = 0;
    break;
  }
  case INPUT_FILE: {
    // Receive the input file, submit it to the correct queue.
    // We expect a string with the queue name, a second string with the
    // input file name and then a third string with the input file.
    QString strings[4];
    in >> strings[0] >> strings[1] >> strings[2] >> strings[3];
    qDebug() << strings[0];
    qDebug() << strings[1];
    qDebug() << strings[2];
    qDebug() << strings[3];
    emit(jobSubmitted(strings[0], strings[1], strings[2], strings[3]));
    m_blockSize = 0;
    m_state = IDLE;
    break;
  }
  case OUTPUT_FILE:
  case ERROR:
  default:
    qDebug() << Q_FUNC_INFO << "Unhandled socket state:" << m_state;
    break;
  }

  m_blockSize = 0;

}

void Connection::sendMessage(const QString &message)
{
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_7);
  out << static_cast<quint16>(0);
  out << message;

  //QList<QString> list;
  //list << "GAMESS" << "MOPAC";
  //out << list;

  out.device()->seek(0);
  out << static_cast<quint16>(block.size() - sizeof(quint16));

  qDebug() << "size:" << block.size() << sizeof(quint16) << "message:" << block;

  m_socket->write(block);
}

} // End namespace
