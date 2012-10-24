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

#ifndef DIRLISTINGINFO_H_
#define DIRLISTINGINFO_H_

#include "fileinfo.h"

#include <QtCore/QList>
#include <QtCore/QString>

namespace MoleQueue {
namespace Uit {

/**
 * @brief class used to model UIT DirListingInfo.
 */
class DirListingInfo
{
public:
  DirListingInfo();
  /**
   * @param other The instance being copied.
   */
  DirListingInfo(const DirListingInfo &other);

  /**
   * Convert XML representation into object model.
   *
   * @return The DirListingInfo instance representing the XML
   * @param xml The XML to parse.
   */
  static DirListingInfo fromXml(const QString &xml);

  /**
   * @return The current directory.
   */
  QString currentDirectory() const {
    return m_currentDirectory;
  }
  /**
   * @param current The current directory.
   */
  void setCurrentDirectory(const QString& current) {
    m_currentDirectory = current;
  }

  /**
   * @return The list of directories.
   */
  const QList<FileInfo>& directories() const {
    return m_directories;
  }
  /**
   * @param dirs The list of directories.
   */
  void setDirectories(const QList<FileInfo>& dirs) {
    m_directories = dirs;
  }

  /**
   * @return The list of files.
   */
  const QList<FileInfo>& files() const {
    return m_files;
  }
  /**
   * @param files The list of files.
   */
  void setFiles(const QList<FileInfo>& fs) {
    m_files = fs;
  }

  /**
   * @returns true if this DirListingInfo object represents a valid XML
   * document, false otherwise.
   */
  bool isValid() const {
    return m_valid;
  }

  /**
   * @return The raw XML used to generate this object.
   */
  QString xml() {
    return m_xml;
  }

private:
  bool m_valid;
  QString m_currentDirectory;
  QList<FileInfo> m_directories;
  QList<FileInfo> m_files;
  QString m_xml;

  /**
   * Parse the XML and set the fields using the data in the XML document.
   *
   * @param xml The XML document to parse.
   */
  void setContent(const QString &xml);
};

} /* namespace Uit */
} /* namespace MoleQueue */
#endif /* DIRLISTINGINFO_H_ */
