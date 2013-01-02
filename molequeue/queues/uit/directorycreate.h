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

#ifndef UITDIRECTORYCREATE_H_
#define UITDIRECTORYCREATE_H_

#include "filesystemoperation.h"

#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QQueue>
#include <QtCore/QFileInfo>

namespace MoleQueue {
namespace Uit {

class Session;

/**
 * @brief File system operation to create a directory on a UIT host.
 */
class DirectoryCreate: public FileSystemOperation
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  DirectoryCreate(Session *session, QObject *parentObject = 0);

  void start();

  /**
   * @return The directory to create.
   */
  QString directory() const
  {
    return m_directory;
  }

  /**
   * @param dir The directory to create.
   */
  void setDirectory(const QString& dir)
  {
    m_directory = dir;
  }

private slots:
  /**
   * Process the next part of the path.
   */
  void createNext();
  /**
   * Slot called when the current operation is complete.
   */
  void createDirectoryComplete();
  void processStatResponse();
  void statError(const QString &errorString);

private:
  QString m_directory;
  // The individual parts of the directory.
  QStringList m_parts;
  // The current directory being created.
  QString m_currentDirectory;

  void createDirectory(const QString &dir);
};

} /* namespace Uit */
} /* namespace MoleQueue */
#endif /* UITDIRECTORYCREATE_H_ */
