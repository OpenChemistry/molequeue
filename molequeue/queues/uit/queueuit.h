/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef QUEUEUIT_H
#define QUEUEUIT_H
#include "wsdl_uitapi.h"
#include "credentialsdialog.h"
#include "authresponseprocessor.h"
#include "authenticateresponse.h"
#include "../remote.h"

#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QAbstractXmlReceiver>
#include <QtCore/QList>
#include <QtCore/QMutex>

class QueueUitTest;

namespace MoleQueue
{

/** @brief QueueRemote subclass for interacting with a remote queue
 *  over UIT.
 */
class QueueUit : public QueueRemote
{
  Q_OBJECT
public:
  explicit QueueUit(QueueManager *parentManager = 0);
  ~QueueUit();

  virtual QString typeName() const { return "ezHPC UIT"; }

  bool writeJsonSettings(Json::Value &root, bool exportOnly,
                         bool includePrograms) const;

  bool readJsonSettings(const Json::Value &root, bool importOnly,
                        bool includePrograms);

  AbstractQueueSettingsWidget* settingsWidget();

  QString hostName()
  {
    return m_hostName;
  }

  void setHostName(const QString &host) {
    m_hostName = host;
  }

  /**
   * @return The Kerberos username.
   */
  QString kerberosPrinciple() {
    return m_kerberosPrinciple;
  }

  /**
   * Set the Kereros username.
   */
  void setKerberosPrinciple(const QString &principle) {
    m_kerberosPrinciple = principle;
  }

  /**
   * Test the connection to UIT.
   */
  bool testConnection(QWidget *parentObject);

  // Needed for testing
  friend class ::QueueUitTest;

  static const QString clientId;

public slots:
  void requestQueueUpdate();

protected slots:

  void createRemoteDirectory(MoleQueue::Job job);
  void remoteDirectoryCreated();
  void copyInputFilesToHost(MoleQueue::Job job);
  void inputFilesCopied();
  void submitJobToRemoteQueue(MoleQueue::Job job);
  void jobSubmittedToRemoteQueue();
  void handleQueueUpdate();

  void beginFinalizeJob(MoleQueue::IdType queueId);
  void finalizeJobCopyFromServer(MoleQueue::Job job);
  void finalizeJobOutputCopiedFromServer();
  void finalizeJobCopyToCustomDestination(MoleQueue::Job job);
  void finalizeJobCleanup(MoleQueue::Job job);

  void cleanRemoteDirectory(MoleQueue::Job job);
  void remoteDirectoryCleaned();

  void beginKillJob(MoleQueue::Job job);
  void endKillJob();

  /**
   * Called when test authentication is complete
   *
   * @param The session token.
   */
  void testConnectionComplete(const QString &token);

  /**
   * Called when test authentication produces an error
   *
   * @param The error message.
   */
  void testConnectionError(const QString &errorMessage);

private:
  QString m_hostName;
  QString m_kerberosPrinciple;
  UitapiService m_uit;
  QWidget *m_dialogParent;
};

} // End namespace

#endif // QUEUEUIT_H