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

#include "directorydelete.h"
#include "requests.h"
#include "logger.h"
#include "session.h"
#include "filestreamingdata.h"

#include <QtCore/QDir>
#include <QtCore/QBuffer>


namespace MoleQueue {
namespace Uit {

DirectoryDelete::DirectoryDelete(Session *session,
                                 QObject *parentObject)
  : FileSystemOperation(session, parentObject)
{

}

void DirectoryDelete::start()
{
  m_dirsToProcess.push(m_directory);
  deleteNext();
}

void DirectoryDelete::processDirectoryListing()
{
  GetDirectoryListingRequest *request
    = qobject_cast<GetDirectoryListingRequest*>(sender());

  if (!request) {
    QString msg = tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not GetDirectoryListingRequest!");
    Logger::logError(msg);
    emit error(msg);
    return;
  }

  request->deleteLater();

  DirListingInfo info = request->dirListingInfo();

  m_processedDirs.push(info.currentDirectory());

  // Process files
  foreach(const FileInfo& file, info.files()) {
    // enqueue file include full path ...
    m_files.append(info.currentDirectory() + "/" + file.name());
  }

  // Process directories
  foreach(const FileInfo& dir, info.directories()) {
    // enqueue directory include full path ...
    if (dir.name() != "." && dir.name() != "..")
      m_dirsToProcess.push(info.currentDirectory() + "/" + dir.name());
  }

  deleteNext();
}

void DirectoryDelete::deleteNext()
{
  Request *request
    = qobject_cast<Request*>(sender());

  if (request)
    request->deleteLater();

  // Delete all file in a directory first
  if (!m_files.isEmpty()) {
    QString remoteFilePath = m_files.takeFirst();

    DeleteFileRequest *deleteRequest = new DeleteFileRequest(m_session, this);

    deleteRequest->setHostId(m_hostID);
    deleteRequest->setUserName(m_userName);
    deleteRequest->setFile(remoteFilePath);

    connect(request, SIGNAL(finished()),
            this, SLOT(deleteNext()));
    connect(request, SIGNAL(error(const QString &)),
            this, SLOT(requestError(const QString &)));

    deleteRequest->submit();
  }
  // Do we have more directories to explorer
  else if (!m_dirsToProcess.isEmpty()) {
    QString remoteDirPath = m_dirsToProcess.pop();

    GetDirectoryListingRequest *listRequest
      = new GetDirectoryListingRequest(m_session, this);

    listRequest->setDirectory(remoteDirPath);
    listRequest->setHostId(m_hostID);
    listRequest->setUserName(m_userName);

    connect(listRequest, SIGNAL(finished()),
            this, SLOT(processDirectoryListing()));
    connect(listRequest, SIGNAL(error(const QString &)),
            this, SLOT(requestError(const QString &)));

    listRequest->submit();
  }
  // Finally clean up all the directories
  else if (!m_processedDirs.isEmpty()) {
    QString remoteDirPath = m_processedDirs.pop();

    DeleteDirectoryRequest *deleteRequest
      = new DeleteDirectoryRequest(m_session, this);

    deleteRequest->setHostId(m_hostID);
    deleteRequest->setUserName(m_userName);
    deleteRequest->setDirectory(remoteDirPath);

    connect(deleteRequest, SIGNAL(finished()),
            this, SLOT(deleteNext()));
    connect(deleteRequest, SIGNAL(error(const QString &)),
            this, SLOT(requestError(const QString &)));

    deleteRequest->submit();
  }
  else {
    emit finished();
  }
}

} /* namespace Uit */
} /* namespace MoleQueue */
