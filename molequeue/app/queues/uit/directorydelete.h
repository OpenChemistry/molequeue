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

#ifndef UITDIRDELETER_H_
#define UITDIRDELETER_H_

#include "filesystemoperation.h"

#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QStack>
#include <QtCore/QFileInfo>

namespace MoleQueue {
namespace Uit {

class Session;

/**
 * @brief File system operation to delete a directory on a remote UIT system.
 */
class DirectoryDelete : public FileSystemOperation
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  DirectoryDelete(Session *session, QObject *parentObject);

  /**
   * @return The directory being deleted.
   */
  QString directory() const
  {
    return m_directory;
  }

  /**
   * @param dir The directory to delete.
   */
  void setDirectory(const QString& dir)
  {
    m_directory = dir;
  }

  void start();

private slots:
  /**
   * Slot to perform next delete operation.
   */
  void deleteNext();
  /**
   * Slot called to process the directory listing request from UIT.
   */
  void processDirectoryListing();

private:
  QString m_directory;
  // File currently being deleted.
  QList<QString> m_files;
  // Directories that still need to be processed.
  QStack<QString> m_dirsToProcess;
  // Directories that have been processed i.e. a directory listing request has
  // been made.
  QStack<QString> m_processedDirs;
};

} /* namespace Uit */
} /* namespace MoleQueue */
#endif /* UITDIRDELETER_H_ */
