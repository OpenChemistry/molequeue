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

#include <client.h>

#include <QtCore/QCoreApplication>

class Killer : public MoleQueue::Client
{
  Q_OBJECT
public:
  Killer() : MoleQueue::Client() {}
  ~Killer() {}
  void sendRpcKill()
  {
    QJsonObject request;
    emptyRequest(request);
    request["method"] = QLatin1String("rpcKill");
    sendRequest(request);
  }
};

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  QString socketName = "MoleQueue";

  QStringList args = QCoreApplication::arguments();
  for (QStringList::const_iterator it = args.constBegin() + 1,
       itEnd = args.constEnd(); it != itEnd; ++it) {
    if (*it == "-s")
      socketName = *(++it);
  }

  Killer killer;
  killer.connectToServer(socketName);
  if (killer.isConnected())
    killer.sendRpcKill();
  else
    return 1;

  return 0;
}

#include "sendRpcKill.moc"
