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

#include "dummyjsonrpc.h"

DummyJsonRpc::DummyJsonRpc(QObject *p) :
  MoleQueue::JsonRpc(p)
{
}

int DummyJsonRpc::mapMethodNameToInt(const QString &) const
{
  return MoleQueue::JsonRpc::UNRECOGNIZED_METHOD;
}

void DummyJsonRpc::handlePacket(int, PacketForm, MoleQueue::Connection *,
                                const MoleQueue::EndpointId,
                                const Json::Value &)
{
}
