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

#include "messagehandler.h"
#include "logger.h"

namespace MoleQueue {
namespace Uit {

MessageHandler::MessageHandler(QObject *parentObject)
  : QAbstractMessageHandler(parentObject)
{

}

void MessageHandler::handleMessage(QtMsgType type, const QString &description,
                                   const QUrl &identifier,
                                   const QSourceLocation &sourceLocation)
{
  Q_UNUSED(type);
  Q_UNUSED(identifier);
  Q_UNUSED(sourceLocation);
  Logger::logError("UIT XML parse error: " + description);
}

} /* namespace MoleQueue */
} /* namespace MoleQueue */
