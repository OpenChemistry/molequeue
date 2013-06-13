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

#ifndef UITDIRDOWNLOADER_H_
#define UITDIRDOWNLOADER_H_

#include "filesystemoperation.h"

#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QQueue>
#include <QtCore/QFileInfo>

namespace MoleQueue {
namespace Uit {

class Session;

/**
 * @brief File system operation to download a directory from a remote UIT system.
 */
class DirectoryDownload : public FileSystemOperation
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  DirectoryDownload(Session *session, QObject *parentObject);

  /**
   * @return The remote path being downloaded.
   */
  QString remotePath() const
  {
    return m_remotePath;
  }

  /**
   * @param path The remote path to be downloaded.
   */
  void setRemotePath(const QString& path)
  {
    m_remotePath = path;
  }

  /**
   * @return The local path to download the directory to.
   */
  QString localPath() const
  {
    return m_localPath;
  }

  /**
   * @param path The local path to download the directory to.
   */
  void setLocalPath(const QString& path)
  {
    m_localPath = path;
  }

  /**
   * @return The download URL.
   */
  QString url() const
  {
    return m_url;
  }

  /**
   * @param The download URL to use.
   */
  void setUrl(const QString& u)
  {
    m_url = u;
  }

  void start();

private slots:
  void download(const QString &dir);
  void downloadInternal();
  void downloadNext();
  /**
   * Slot to process a directory listing.
   */
  void processDirectoryListing();
  /**
   * Slot called when the current download request is complete.
   *
   * @param reply The reply used to read the file contents.
   */
  void finished(QNetworkReply *reply);

private:
  QString m_remotePath;
  QString m_localPath;
  QNetworkAccessManager *m_networkAccess;
  QString m_url;
  // Queue of directories to download.
  QQueue<QString> m_directories;
  // Queue of files to download.
  QQueue<QString> m_files;
  // The current local file path to write data to.
  QString m_currentFilePath;
};

} /* namespace Uit */
} /* namespace MoleQueue */
#endif /* UITDIRDOWNLOADER_H_ */
