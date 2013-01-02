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

#ifndef UITOPERATION_H_
#define UITOPERATION_H_

#include "job.h"

#include <QtCore/QObject>

namespace MoleQueue {
namespace Uit {

class Session;

/**
 * @brief abstract base class of UIT file system operations.
 *
 * See DirectoryDownload and DirectoryUpload for examples of concrete
 * implementations.
 */
class FileSystemOperation: public QObject
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session to use for this operation.
   * @param parentObject The parent object.
   */
  FileSystemOperation(Session *session, QObject *parentObject = 0);

  /**
   * @return The host ID for the host this operation associated with.
   */
  qint64 hostId() const
  {
    return m_hostID;
  }

  /**
   * @param id The host ID for the host this operation is associated with.
   */
  void setHostId(qint64 id)
  {
    m_hostID = id;
  }

  /**
   * @return The user name of the user performing this file system operation.
   */
  QString userName() const
  {
    return m_userName;
  }

  /**
   * @param user The user name of the user performing this file system
   * operation.
   */
  void setUserName(const QString& user)
  {
    m_userName = user;
  }

  /**
   * @return The MoleQueue job this operation is associated with.
   */
  const Job& job() const
  {
    return m_job;
  }
  /**
   * @param j The MoleQueue job this operation is associated with.
   */
  void setJob(const Job& j)
  {
    this->m_job = j;
  }

  /**
   * Implemented by subclasses, start the operation.
   */
  virtual void start() = 0;

  /**
   * Error string produced by UIT statFile(...) when an file/directory doesn't
   * exist.
   */
  static const QString noSuchFileOrDir;

signals:
  /**
   * Emitted when the operation is complete.
   */
  void finished();
  /**
   * Emitted if an error occurs during the operation.
   *
   * @param errorString The error string describing the error.
   */
  void error(const QString &errorString);

protected slots:
  /**
   * Slot called when an error occurs while performing the operation.
   *
   * @param errorString String describing the error.
   */
  virtual void requestError(const QString &errorString);

protected:
  Session *m_session;
  qint64 m_hostID;
  QString m_userName;
  Job m_job;

};

} /* namespace Uit */
} /* namespace MoleQueue */
#endif /* UITOPERATION_H_ */
