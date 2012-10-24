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

#ifndef UITFILEUPLOADER_H_
#define UITFILEUPLOADER_H_

#include "filesystemoperation.h"

#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QQueue>
#include <QtCore/QFileInfo>

namespace MoleQueue {
namespace Uit {

class Session;

/**
 * @brief File system operation to upload a directory to a remote UIT system.
 */
class DirectoryUpload: public FileSystemOperation
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  DirectoryUpload(Session *session, QObject *parentObject = 0);

  /**
   * @return The local path to be uploaded.
   */
  QString localPath() const {
    return m_localPath;
  }
  /**
   * @param path The local path to be uploaded.
   */
  void setLocalPath(const QString& path) {
    m_localPath = path;
  }
  /**
   * @return The remote file path for the directory to be uploaded to.
   */
  QString targetPath() const {
    return m_remotePath;
  }
  /**
   * @param path The target path on the remote system.
   */
  void setRemotePath(const QString& path) {
    m_remotePath = path;
  }

  void start();

private slots:
  void uploadInternal();
  void uploadNext();
  void uploadFile(const QFileInfo &fileInfo);
  void finished(QNetworkReply *reply);
  void createDirectoryComplete();

private:
  QString m_localPath;
  QString m_remotePath;
  QNetworkAccessManager *m_networkAccess;
  QString m_url;
  QQueue<QFileInfo> m_fileEntries;

  void uploadFile(const QString &filePath);


};

} /* namespace Uit */
} /* namespace MoleQueue */
#endif /* UITFILEUPLOADER_H_ */
