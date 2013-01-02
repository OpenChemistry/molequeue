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

#include "directorycreate.h"
#include "requests.h"
#include "filestreamingdata.h"
#include "logger.h"
#include "session.h"

#include <QtCore/QDir>
#include <QtCore/QBuffer>

namespace MoleQueue {
namespace Uit {


DirectoryCreate::DirectoryCreate(Session *session,
                                       QObject *parentObject)
  : FileSystemOperation(session, parentObject)
{

}

void DirectoryCreate::start()
{
  if (m_directory.isEmpty()) {
    Logger::logWarning("Trying to create empty directory!");
    return;
  }

  m_parts = m_directory.split("/");
  m_parts.removeAll("");

  if (m_directory[0] == '/')
    m_currentDirectory = "/";

  createNext();
}

void DirectoryCreate::createNext()
{
  if (m_parts.isEmpty()) {
    emit finished();
    return;
  }

  if (!m_currentDirectory.isEmpty() &&
      m_currentDirectory[m_currentDirectory.length()-1] != '/')
    m_currentDirectory += "/";

  m_currentDirectory += m_parts.takeFirst();

  StatFileRequest *statRequest = new StatFileRequest(m_session, this);
  statRequest->setHostId(m_hostID);
  statRequest->setUserName(m_userName);
  statRequest->setFilename(m_currentDirectory);

  connect(statRequest, SIGNAL(finished()),
          this, SLOT(processStatResponse()));
  connect(statRequest, SIGNAL(error(const QString &)),
          this, SLOT(statError(const QString &)));

  statRequest->submit();
}

void DirectoryCreate::processStatResponse()
{
  StatFileRequest *request = qobject_cast<StatFileRequest*>(sender());
  if (!request) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not StatFileRequest!"));
    return;
  }

  request->deleteLater();

  createNext();
}

void DirectoryCreate::statError(const QString &errorString)
{
  if (errorString.contains(FileSystemOperation::noSuchFileOrDir))
    createDirectory(m_currentDirectory);
  else
    requestError(errorString);
}

void DirectoryCreate::createDirectory(const QString &dir) {

  CreateDirectoryRequest *request = new CreateDirectoryRequest(m_session, this);

  request->setHostId(m_hostID);
  request->setUserName(m_userName);
  request->setDirectory(dir);

  connect(request, SIGNAL(finished()),
          this, SLOT(createDirectoryComplete()));
  connect(request, SIGNAL(error(const QString &)),
          this, SLOT(requestError(const QString &)));

  request->submit();
}

void DirectoryCreate::createDirectoryComplete()
{
  CreateDirectoryRequest *request
      = qobject_cast<CreateDirectoryRequest*>(sender());

  if (!request) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not CreateDirectoryRequest!"));
    return;
  }
  request->deleteLater();

  createNext();
}

} /* namespace Uit */
} /* namespace MoleQueue */
