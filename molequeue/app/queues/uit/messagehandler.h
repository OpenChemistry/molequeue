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

#ifndef MESSAGEHANDLER_H_
#define MESSAGEHANDLER_H_

#include <QtXmlPatterns/QAbstractMessageHandler>

namespace MoleQueue {
namespace Uit {

/**
 * Concrete QAbstractMessageHandler implementation used to report errors
 * associated with parsing XML content past QXmlQuery objects.
 */
class MessageHandler : public QAbstractMessageHandler
{
public:
  MessageHandler(QObject *parentObject = 0);

protected:
  virtual void handleMessage(QtMsgType type, const QString &description,
                             const QUrl &identifier,
                             const QSourceLocation &sourceLocation);
};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* MESSAGEHANDLER_H_ */
