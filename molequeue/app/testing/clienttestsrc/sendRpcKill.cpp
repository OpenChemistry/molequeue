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

#include <jsonrpcclient.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>

class Killer : public MoleQueue::JsonRpcClient
{
  Q_OBJECT
public:
  Killer(QObject *p = 0) : MoleQueue::JsonRpcClient(p) {}
  ~Killer() {}
  void sendRpcKill()
  {
    QJsonObject request = emptyRequest();
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

  Killer killer(&app);
  killer.connectToServer(socketName);
  if (!killer.isConnected())
    return 1;

  killer.sendRpcKill();
  killer.flush();
  return 0;
}

#include "sendRpcKill.moc"
