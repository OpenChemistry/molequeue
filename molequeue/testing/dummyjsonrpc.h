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

#ifndef DUMMYJSONRPC_H
#define DUMMYJSONRPC_H

#include "transport/jsonrpc.h"

namespace MoleQueue {
class Message;
}

class DummyJsonRpc : public MoleQueue::JsonRpc
{
  Q_OBJECT
public:
  explicit DummyJsonRpc(QObject *p = 0);

protected:
  int mapMethodNameToInt(const QString &methodName) const;
  void handleMessage(int method, const MoleQueue::Message &);
};

#endif // DUMMYJSONRPC_H
