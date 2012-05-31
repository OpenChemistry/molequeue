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

#include "server.h"

#include "job.h"
#include "jobmanager.h"
#include "queuemanager.h"
#include "serverconnection.h"

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QList>

#define DEBUGOUT(title) \
  if (this->m_debug)    \
    qDebug() << QDateTime::currentDateTime().toString() \
             << "Server" << title <<

namespace MoleQueue
{

Server::Server(QObject *parentObject)
  : QObject(parentObject),
    m_server(new QLocalServer (this)),
    m_jobManager(new JobManager (this)),
    m_queueManager(new QueueManager (this)),
    m_isTesting(false),
    m_debug(false)
{
  qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
  qRegisterMetaType<ServerConnection*>("ServerConnection*");

  connect(m_server, SIGNAL(newConnection()),
          this, SLOT(newConnectionAvailable()));

  connect(m_jobManager, SIGNAL(jobAboutToBeAdded(Job*)),
          this, SLOT(jobAboutToBeAdded(Job*)),
          Qt::DirectConnection);
}

Server::~Server()
{
  this->stop();

  qDeleteAll(m_connections);
  m_connections.clear();

  delete m_server;
  m_server = NULL;

  delete m_jobManager;
  m_jobManager = NULL;

  delete m_queueManager;
  m_queueManager = NULL;
}

void Server::start()
{
  const QString serverName = (!m_isTesting) ? "MoleQueue"
                                            : "MoleQueue-testing";
  if (!m_server->listen(serverName)) {
    DEBUGOUT("start") "Error starting local socket server. Error type:"
        << m_server->serverError() << m_server->errorString();
    emit connectionError(m_server->serverError(), m_server->errorString());
    return;
  }

  DEBUGOUT("start") "Local socket server started listening on address:"
      << m_server->serverName();
}

void Server::forceStart()
{
  const QString serverName = (!m_isTesting) ? "MoleQueue"
                                            : "MoleQueue-testing";
  DEBUGOUT("forceStart") "Attempting to remove existing servers...";
  if (m_server->removeServer(serverName)) {
    DEBUGOUT("forceStart") "Servers removed.";
  }
  else {
    DEBUGOUT("forceStart") "Failed to remove existing servers.";
  }

  this->start();
}

void Server::stop()
{
  m_server->close();
}

void Server::jobAboutToBeAdded(Job *job)
{
  job->setMolequeueId(static_cast<IdType>(m_jobManager->count()) + 1);
}

void Server::newConnectionAvailable()
{
  if (!m_server->hasPendingConnections()) {
    DEBUGOUT("newConnectionAvailable") "Aborting -- no pending connections "
        "available.";
    return;
  }

  QLocalSocket *socket = m_server->nextPendingConnection();
  ServerConnection *conn = new ServerConnection (this, socket);

  /// @todo make connections between the new ServerConnection and the rest of
  /// the MoleQueue application here.

  m_connections.append(conn);
  connect(conn, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
  emit newConnection(conn);
  DEBUGOUT("newConnectionAvailable") "New connection added:" << conn;
  conn->startProcessing();
}

void Server::clientDisconnected()
{
  ServerConnection *conn = qobject_cast<ServerConnection*>(this->sender());
  if (conn == NULL) {
    qWarning() << Q_FUNC_INFO << "called without a ServerConnection as sender.";
    return;
  }

  DEBUGOUT("clientDisconnected") "Removing connection" << conn;
  m_connections.removeOne(conn);
  conn->deleteLater();
}

} // end namespace MoleQueue
