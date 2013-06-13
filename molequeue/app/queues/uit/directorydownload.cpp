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

#include "directorydownload.h"
#include "requests.h"
#include "logger.h"
#include "session.h"
#include "filestreamingdata.h"
#include "filesystemoperation.h"

#include <QtCore/QDir>
#include <QtCore/QBuffer>

namespace MoleQueue {
namespace Uit {

DirectoryDownload::DirectoryDownload(Session *session,
                                     QObject *parentObject)
: FileSystemOperation(session, parentObject)
{
  m_networkAccess = new QNetworkAccessManager(this);

  connect(m_networkAccess, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(finished(QNetworkReply*)));
}

void DirectoryDownload::start()
{
  GetStreamingFileDownloadURLRequest *request
    = new GetStreamingFileDownloadURLRequest(m_session, this);

  connect(request, SIGNAL(finished()),
          this, SLOT(downloadInternal()));
  connect(request, SIGNAL(error(const QString &)),
          this, SLOT(requestError(const QString &)));

  request->submit();
}

void DirectoryDownload::downloadInternal()
{
  GetStreamingFileDownloadURLRequest *request
    = qobject_cast<GetStreamingFileDownloadURLRequest*>(sender());

  if (!request) {
    QString msg = tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                  .arg("Sender is not GetStreamingFileDownloadURLRequest!");
    Logger::logError( msg);
    emit error(msg);
    return;
  }

  request->deleteLater();
  m_url = request->url();
  m_directories.enqueue(m_remotePath);

  downloadNext();
}

void DirectoryDownload::download(const QString &dir)
{
  GetDirectoryListingRequest *request
    = new GetDirectoryListingRequest(m_session, this);

  request->setDirectory(dir);
  request->setHostId(m_hostID);
  request->setUserName(m_userName);

  connect(request, SIGNAL(finished()),
          this, SLOT(processDirectoryListing()));
  connect(request, SIGNAL(error(const QString &)),
          this, SLOT(requestError(const QString &)));

  request->submit();
}

void DirectoryDownload::processDirectoryListing()
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

  if (!info.isValid()) {
    QString msg = tr("Invalid response from UIT server: %1")
                            .arg(info.xml());
    Logger::logError(msg);
    emit error(msg);
    return;
  }

  // Process directories
  foreach(const FileInfo& dir, info.directories()) {
    // enqueue directory include full path ...
    if (dir.name() != "." && dir.name() != "..")
      m_directories.enqueue(info.currentDirectory() + "/" + dir.name());
  }

  // Process files
  foreach(const FileInfo& file, info.files()) {
    // enqueue directory include full path ...
    m_files.enqueue(info.currentDirectory() + "/" + file.name());
  }

  downloadNext();
}

void DirectoryDownload::downloadNext()
{
  // Process all files in a directory first
  if (!m_files.isEmpty()) {
    QString remoteFilePath = m_files.dequeue();
    QString localFilePath = remoteFilePath;
    localFilePath = m_localPath + localFilePath.replace(m_remotePath, "");


    // Ensure the directory exists
    QString path = QFileInfo(localFilePath).path();
    if (!QDir(m_localPath).mkpath(path)) {
      QString msg = tr("Unable to create directory: %1").arg(path);
      Logger::logError(msg);
      emit error(msg);
      return;
    }

    // Save the file path so we know where to write the data.
    m_currentFilePath = localFilePath;

    // Now transfer the file
    FileStreamingData fileData;
    fileData.setToken(m_session->token());
    fileData.setFileName(remoteFilePath);
    fileData.setUserName(m_userName);
    fileData.setHostID(m_hostID);

    QString xml = fileData.toXml();

    QByteArray bytes;
    QTextStream stream(&bytes);

    stream << xml.size();
    stream << "|";
    stream << xml;
    stream.flush();

    QNetworkRequest request;
    request.setUrl(QUrl(m_url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QVariant("application/xml"));
    m_networkAccess->post(request, bytes);
  }
  else if (!m_directories.isEmpty()) {
    QString nextDir = m_directories.dequeue();
    download(nextDir);
  }
  else {
    emit FileSystemOperation::finished();
  }
}

void DirectoryDownload::finished(QNetworkReply *reply)
{
  if (reply->error() == QNetworkReply::NoError)
  {
    QFile file(m_currentFilePath);
    if (!file.open(QFile::WriteOnly)) {
      QString msg = tr("Unable to open file for write: %1")
                              .arg(m_currentFilePath);
      Logger::logError(msg);
      emit error(msg);
      return;
    }

    qint64 bytesRead = -1;
    char bytes[4048];

    while((bytesRead = reply->read(bytes, 4048)) != -1) {
      file.write(bytes, bytesRead);
    }

    file.close();

    downloadNext();
  }
  else
  {
    Logger::logError(tr("Error downloading file: %1")
                          .arg(reply->errorString()), m_job.moleQueueId());

    emit error(reply->errorString());
  }
}

} /* namespace Uit */
} /* namespace MoleQueue */
