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

#ifndef DUMMYCONNECTIONLISTENER_H
#define DUMMYCONNECTIONLISTENER_H

#include "transport/connectionlistener.h"

class DummyConnection;

class DummyConnectionListener : public MoleQueue::ConnectionListener
{
  Q_OBJECT
public:
  explicit DummyConnectionListener(QObject *aparent = 0);

  /// Emit @a conn as a new connection from this listener.
  void emitNewConnection(DummyConnection *conn);

  // Reimplemented from base class
  void start();
  void stop(bool force);
  void stop();
  QString connectionString() const;
};

#endif // DUMMYCONNECTIONLISTENER_H
