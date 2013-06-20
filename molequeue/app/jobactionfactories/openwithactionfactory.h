/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2012-2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MOLEQUEUE_OPENWITHACTIONFACTORY_H
#define MOLEQUEUE_OPENWITHACTIONFACTORY_H

#include "../jobactionfactory.h"

#include <molequeue/servercore/molequeueglobal.h>

class QDir;

namespace MoleQueue
{

/**
 * @class OpenWithActionFactory openwithactionfactory.h
 * <molequeue/jobactionfactories/openwithactionfactory.h>
 * @brief The OpenWithActionFactory class provides a generic mechanism for
 * performing an action on a file in a job's directory. It is configured to
 * process a file by calling an external executable or sending RPC requests.
 *
 * The OpenWithActionFactory allows arbitrary actions to be performed on files
 * in a job's directory. A list of QRegExp objects is used to filter filenames
 * so that the factory only produces actions for files that match one of the
 * filePatterns().
 *
 * The actions will either call an external executable() to handle the file,
 * or send a request (rpcMethod()) to a JSON-RPC 2.0 server (rpcServer()). The
 * executable will be run as:
~~~
executable /absolute/path/to/selected/fileName
~~~
 *
 * RPC requests will be of the form:
~~~
{
    "jsonrpc": "2.0",
    "method": "rpcMethod",
    "params": {
        "fileName": "/absolute/path/to/selected/fileName"
        }
    },
    "id": "XXX"
}
~~~
 * Use setExecutable() to set the actions to use an executable, or
 * setRpcDetails() to use RPC calls. The type of file handler can be checked
 * with handlerType().
 */
class OpenWithActionFactory : public JobActionFactory
{
  Q_OBJECT
public:
  /**
   * The HandlerType enum identifies types of file handling strategies.
   * @sa handlerType()
   */
  enum HandlerType {
    /**
     *No handler specified.
     */
    NoHandler = -1,
    /**
     * Open the file with an external executable.
     * @sa setExecutable().
     */
    ExecutableHandler = 0,
    /**
     * Open the file with a JSON-RPC request.
     * @sa setRpcDetails().
     */
    RpcHandler
  };

  /**
   * Construct a new, uninitialized OpenWithActionFactory.
   */
  OpenWithActionFactory();

  virtual ~OpenWithActionFactory();

  /**
   * Construct a copy of the OpenWithActionFactory @a other.
   */
  OpenWithActionFactory(const OpenWithActionFactory &other);

  /**
   * Copy the OpenWithActionFactory @a other into @a this.
   */
  OpenWithActionFactory & operator=(OpenWithActionFactory other);

  /**
   * Save/restore state. @{
   */
  void readSettings(QSettings &settings);
  void writeSettings(QSettings &settings) const;
  /** @} */

  /**
   * The user-friendly GUI name of this action. Used to set the action menu text
   * to "Open '[job description]' with '[name()]'". @{
   */
  QString name() const { return m_name; }
  void setName(const QString &n) { m_name = n; }
  /** @} */

  // Reimplemented virtuals:
  bool isValidForJob(const Job &job) const;
  void clearJobs();
  bool useMenu() const;
  QString menuText() const;
  QList<QAction *> createActions();
  unsigned int usefulness() const;

  /**
   * The type of file handling strategy to use.
   * @sa HandlerType
   * @sa setExecutable() setRpcDetails()
   * @{
   */
  void setHandlerType(HandlerType type);
  HandlerType handlerType() const { return m_handlerType; }
  /** @} */

  /**
   * Produce actions that execute @a exec on the selected file as:
~~~
executable /absolute/path/to/selected/fileName
~~~
   *
   * @note Calling setExecutable() erases the rpcServer() and rpcMethod()
   * values.
   * @{
   */
  void setExecutable(const QString &exec);
  QString executable() const;
  /** @} */

  /**
   * Produce actions that set JSON-RPC 2.0 requests to a local socket server
   * named @a myRpcServer of the form:
~~~
{
    "jsonrpc": "2.0",
    "method": "myRpcMethod",
    "params": {
        "fileName": "/absolute/path/to/selected/fileName"
        }
    },
    "id": "XXX"
}
~~~
   * @note This method erases the executable() value.
   */
  void setRpcDetails(const QString &myRpcServer, const QString &myRpcMethod);

  /**
   * @return The target JSON-RPC server socket name.
   */
  QString rpcServer() const;

  /**
   * @return The method to use in JSON-RPC requests.
   */
  QString rpcMethod() const;

  /**
   * A list of QRegExp objects that match files supported by the file handler.
   * An action will be produce for each file that matches any of these QRegExps.
   * @{
   */
  QList<QRegExp> filePatterns() const;
  QList<QRegExp>& filePatternsRef();
  const QList<QRegExp>& filePatternsRef() const;
  void setFilePatterns(const QList<QRegExp> &patterns);
  /** @} */

  class HandlerStrategy;
private slots:
  void actionTriggered();

private:
  bool scanDirectoryForRecognizedFiles(const QDir &baseDir,
                                       const QDir &dir) const;

  QString m_name;
  QString m_menuText;
  HandlerType m_handlerType;
  HandlerStrategy *m_handler;
  QList<QRegExp> m_filePatterns;
  mutable QMap<QString, QString> m_fileNames; // GUI name: absolute file path
};

} // end namespace MoleQueue

#endif // MOLEQUEUE_OPENWITHACTIONFACTORY_H
