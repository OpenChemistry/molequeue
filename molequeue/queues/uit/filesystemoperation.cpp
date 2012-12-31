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

#include "filesystemoperation.h"
#include "requests.h"

namespace MoleQueue {
namespace Uit {

const QString FileSystemOperation::noSuchFileOrDir = "DIR_LISTING Failed: No such file";

FileSystemOperation::FileSystemOperation(Session *session,
                                               QObject *parentObject)
  : QObject(parentObject), m_session(session), m_hostID(-1)
{

}

void FileSystemOperation::requestError(const QString &errorString)
{
  Request *request
    = qobject_cast<Request*>(sender());

  if (request)
    request->deleteLater();

  emit error(errorString);
}

} /* namespace Uit */
} /* namespace MoleQueue */
