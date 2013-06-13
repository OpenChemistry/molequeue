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

#include "directoryupload.h"
#include "requests.h"
#include "filestreamingdata.h"
#include "logger.h"
#include "session.h"
#include "compositeiodevice.h"

#include <QtCore/QDir>
#include <QtCore/QBuffer>

namespace MoleQueue {
namespace Uit {

DirectoryUpload::DirectoryUpload(Session *session,
                                       QObject *parentObject)
  : FileSystemOperation(session, parentObject)
{
  m_networkAccess = new QNetworkAccessManager(this);

  connect(m_networkAccess, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(finished(QNetworkReply*)));
}

void DirectoryUpload::start()
{
  GetStreamingFileUploadURLRequest *request
    = new GetStreamingFileUploadURLRequest(m_session, this);

  connect(request, SIGNAL(finished()),
          this, SLOT(uploadInternal()));
  connect(request, SIGNAL(error(const QString &)),
          this, SLOT(requestError(const QString &)));

  request->submit();
}

void DirectoryUpload::uploadInternal()
{
  GetStreamingFileUploadURLRequest *request
    = qobject_cast<GetStreamingFileUploadURLRequest*>(sender());

  if (!request) {
    QString msg = tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not GetStreamingFileUploadURLRequest!");
    Logger::logError(msg, m_job.moleQueueId());
    emit error(msg);
    return;
  }

  request->deleteLater();
  m_url = request->url();

  m_fileEntries.enqueue(QFileInfo(m_localPath));
  uploadNext();
}

void DirectoryUpload::uploadNext()
{
  if (m_fileEntries.isEmpty()) {
    emit FileSystemOperation::finished();
    return;
  }

  QFileInfo path = m_fileEntries.dequeue();

  if (path.isDir()) {
    QFileInfoList entries  = QDir(path.absoluteFilePath())
                                 .entryInfoList(QDir::Dirs |
                                                QDir::Files |
                                                QDir::NoDotAndDotDot);

    m_fileEntries.append(entries);

    QString dir = path.absoluteFilePath().replace(m_localPath, "");

    if (!dir.startsWith("/"))
      dir = "/" + dir;

    CreateDirectoryRequest *request = new CreateDirectoryRequest(m_session,
                                                                 this);
    request->setHostId(m_hostID);
    request->setUserName(m_userName);
    request->setDirectory(m_remotePath + dir);

    connect(request, SIGNAL(finished()),
            this, SLOT(createDirectoryComplete()));
    connect(request, SIGNAL(error(const QString &)),
            this, SLOT(requestError(const QString &)));

    request->submit();
 }
 else {
   uploadFile(path);
 }
}

void DirectoryUpload::createDirectoryComplete()
{
  CreateDirectoryRequest *request
      = qobject_cast<CreateDirectoryRequest*>(sender());

  if (!request) {
    QString msg = tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not CreateDirectoryRequest!");
    Logger::logError(msg, m_job.moleQueueId());
    emit error(msg);
    return;
  }
  request->deleteLater();

  uploadNext();
}

void DirectoryUpload::uploadFile(const QFileInfo &fileInfo)
{
  FileStreamingData fileData;

  QString filePath = fileInfo.absoluteFilePath().replace(m_localPath, "");

  if (!filePath.startsWith("/"))
    filePath = "/" + filePath;

  QString remoteFilePath = m_remotePath + filePath;


  fileData.setToken(m_session->token());
  fileData.setFileName(remoteFilePath);
  fileData.setUserName(m_userName);
  fileData.setHostID(m_hostID);

  QString xml = fileData.toXml();

  // Open file and get size
  QFile *file = new QFile(fileInfo.absoluteFilePath());
  if (!file->open(QIODevice::ReadOnly)) {
    QString msg = tr("Unable to open file: %1")
                     .arg(fileInfo.absoluteFilePath());
    Logger::logError(msg, m_job.moleQueueId());
    emit error(msg);
    return;
  }

  CompositeIODevice *dataStream = new CompositeIODevice(this);
  dataStream->open(QIODevice::ReadWrite);
  QBuffer *headerBuffer = new QBuffer(dataStream);
  headerBuffer->open(QIODevice::ReadWrite);
  QTextStream headerStream(headerBuffer);

  headerStream << xml.size();
  headerStream << "|";
  headerStream << xml;
  headerStream << file->size();
  headerStream << "|";
  headerStream.flush();

  headerBuffer->seek(0);
  dataStream->addDevice(headerBuffer);
  dataStream->addDevice(file);

  m_networkAccess->post(QNetworkRequest(QUrl(m_url)), dataStream);
}

void DirectoryUpload::finished(QNetworkReply *reply)
{
  QByteArray response = reply->readAll();

  if (!response.isEmpty() && !QString(response).contains("DONE"))
    Logger::logError(response, m_job.moleQueueId());

  if (reply->error() == QNetworkReply::NoError)
    uploadNext();
  else
    emit error(reply->errorString());
}

} /* namespace Uit */
} /* namespace MoleQueue */
