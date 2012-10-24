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

#ifndef FILEINFO_H_
#define FILEINFO_H_

#include <QtCore/QString>

namespace MoleQueue {
namespace Uit {

/**
 * @brief class used to model UIT FileInfo.
 */
class FileInfo
{
public:
  FileInfo();

  /**
   * @return The date string for this file.
   */
  QString date() const {
    return m_date;
  }
  /**
   * @param d The date string.
   */
  void setDate(const QString& d) {
    m_date = d;
  }

  /**
   * @return The string describing the group the file belongs to.
   */
  QString group() const {
    return m_group;
  }
  /**
   * @param g The string describing the group the file belongs to.
   */
  void setGroup(const QString& g) {
    m_group = g;
  }

  /**
   * @return The name of the file.
   */
  QString name() const {
    return m_name;
  }
  /**
   * @param n The name of the file.
   */
  void setName(const QString& n) {
    m_name = n;
  }

  /**
   * @return The permissions of the file.
   */
  QString perms() const {
    return m_perms;
  }
  /**
   * @param p The string describing the permssion for this file.
   */
  void setPerms(const QString& p) {
    m_perms = p;
  }

  /**
   * @return The size of the file.
   */
  qint64 size() const {
    return m_size;
  }
  /**
   * @param s The size of the file.
   */
  void setSize(qint64 s) {
    m_size = s;
  }

  /**
   * @return The user who owns the file.
   */
  QString user() const {
    return m_user;
  }
  /**
   * @param u The user who owns this file.
   */
  void setUser(const QString& u) {
    m_user = u;
  }

private:
  qint64 m_size;
  QString m_name;
  QString m_perms;
  QString m_date;
  QString m_user;
  QString m_group;

};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* FILEINFO_H_ */
