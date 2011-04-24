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

#include "Connection.h"

#include <QtCore/QDataStream>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QDebug>

namespace MoleQueue {

Connection::Connection(QLocalSocket *socket, QObject *parent) : QObject(parent),
  m_socket(socket), m_blockSize(0)
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

  QString message;
  in >> message;
  qDebug() << "Message:" << message;

  if (m_socket->bytesAvailable()) {
    if (m_socket->bytesAvailable() < static_cast<int>(sizeof(quint16)))
      return;
    in >> m_blockSize;
    if (in.atEnd())
      return;

    in >> message;
    qDebug() << "Message2:" << message;

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
