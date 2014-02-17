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

#ifndef DUMMYSERVER_H
#define DUMMYSERVER_H

#include "server.h"

#include "dummyqueuemanager.h"
#include "server.h"

#include <QtWidgets/QApplication>

#include <QtCore/QDateTime>
#include <QtCore/QThread>
#include <QtCore/QTimer>

/// "Safe" server to use in unit tests. Uses randomized socket name.
class DummyServer : public MoleQueue::Server
{
  Q_OBJECT
public:
  DummyServer(QObject *parentObject = NULL);

  ~DummyServer();

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
    qsrand(seed);
    int randVal = qrand();

    return QString("MoleQueue-testing-%1").arg(QString::number(randVal));
  }

};

#endif // DUMMYSERVER_H
