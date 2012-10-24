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

#ifndef FILESTREAMINGDATA_H_
#define FILESTREAMINGDATA_H_

#include <QtCore/QString>

namespace MoleQueue {
namespace Uit {

/**
 * @brief class used to model UIT FileStreamingData XML document.
 * Used to describe a file on a remote UIT system during the
 * download/upload process.
 */
class FileStreamingData
{
public:
  FileStreamingData();

  /**
   * @return The XML document generated using the fields in this
   * instance. This document can be POSTed to the UIT interface.
   */
  QString toXml() const;

  /**
   * @return The file name.
   */
  QString fileName() const {
    return m_fileName;
  }
  /**
   * @param file The file name.
   */
  void setFileName(const QString& file) {
    m_fileName = file;
  }

  /**
   * @return The host ID for the host this file is associated with.
   */
  qint64 hostID() const {
    return m_hostID;
  }
  /**
   * @param hostId The host ID for the host the file is associatd with.
   */
  void setHostID(qint64 hostId) {
    m_hostID = hostId;
  }

  /**
   * @return The UIT session token.
   */
  QString token() const {
    return m_token;
  }
  /**
   * @param tok The UIT session token to use.
   */
  void setToken(const QString& tok) {
    m_token = tok;
  }

  /**
   * @return The user name for user which owns this file.
   */
  QString userName() const {
    return m_userName;
  }
  /**
   * @param user The user name for the user which owns this file.
   */
  void setUserName(const QString& user) {
  m_userName = user;
  }

private:
  QString m_token;
  QString m_fileName;
  qint64 m_hostID;
  QString m_userName;

};

} /* namespace Uit */
} /* namespace MoleQueue */
#endif /* FILESTREAMINGDATA_H_ */
