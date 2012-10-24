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
#include "uit/authresponseprocessor.h"
#include "uit/authenticateresponse.h"
#include "uit/session.h"
#include "uit/userhostassoclist.h"
#include "uit/jobevent.h"
#include "remote.h"


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

  bool writeJsonSettings(QJsonObject &json, bool exportOnly,
                         bool includePrograms) const;

  bool readJsonSettings(const QJsonObject &json, bool importOnly,
                        bool includePrograms);

  AbstractQueueSettingsWidget* settingsWidget();

  /**
   * @return The Kerberos username.
   */
  QString kerberosUserName() {
    return m_kerberosUserName;
  }

  /**
   * Set the Kereros username.
   */
  void setKerberosUserName(const QString &userName) {
    m_kerberosUserName = userName;
  }

  /**
   * @return The Kerberos realm.
   */
  QString kerberosRealm() {
    return m_kerberosRealm;
  }

  /**
   * Set the Kerberos realm.
   */
  void setKerberosRealm(const QString &realm) {
    m_kerberosRealm = realm;
  }

  QString hostName() const {
    return m_hostName;
  }

  void setHostName(const QString &host) {
    m_hostName = host;
  }

  qint64  hostId() {
    return m_hostID;
  }

  void setHostID(qint64 id) {
    m_hostID = id;
  }

  /**
   * Test the connection to UIT.
   */
  bool testConnection(QWidget *parentObject);

//  /**
//   * Submit sleep job to queue.
//   */
//  void sleepTest(QWidget *parentObject);

  // Needed for testing
  friend class ::QueueUitTest;

  static const QString clientId;

signals:
  void uitMethodError(const QString &errorString);
  void userHostAssocList(const Uit::UserHostAssocList &list);

public slots:
  void requestQueueUpdate();

protected slots:

  void createRemoteDirectory(MoleQueue::Job job);
  void createRemoteDirectoryError(const QString &errorString);
  void remoteDirectoryCreated();

  void copyInputFilesToHost(MoleQueue::Job job);
  void copyInputFilesToHostError(const QString &erroString);
  void inputFilesCopied();
  void uploadInputFilesToHost(Job job);
  void processStatFileRequest();

  void submitJobToRemoteQueue(MoleQueue::Job job);
  void jobSubmittedToRemoteQueue();
  void jobSubmissionError(const QString &errorString);
  void handleQueueUpdate();
  void handleQueueUpdate(const QList<Uit::JobEvent> &jobEvents);
  void requestQueueUpdateError(const QString&);

  //void beginJobSubmission(Job job);
  void beginFinalizeJob(MoleQueue::IdType queueId);
  void finalizeJobCopyFromServer(MoleQueue::Job job);
  void finalizeJobOutputCopiedFromServer();
  void finalizeJobCopyFromServerError(const QString &errorString);
  void finalizeJobCopyToCustomDestination(MoleQueue::Job job);
  void finalizeJobCleanup(MoleQueue::Job job);

  void cleanRemoteDirectory(MoleQueue::Job job);
  void cleanRemoteDirectoryError(const QString &errorString);
  void remoteDirectoryCleaned();

  void beginKillJob(MoleQueue::Job job);
  void killJobError(const QString &errorString);
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
  Uit::Session *m_uitSession;
  QString m_kerberosUserName;
  QString m_kerberosRealm;
  QString m_hostName;
  qint64 m_hostID;
  UitapiService m_uit;
  QWidget *m_dialogParent;
  bool m_isCheckingQueue;

  Uit::Session * uitSession();

private slots:
  void getUserHostAssoc();
  void getUserHostAssocComplete();

  void requestError(const QString &errorMessage);
  JobState jobEventToJobState(Uit::JobEvent event);
};

} // End namespace

#endif // QUEUEUIT_H
