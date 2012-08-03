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

#ifndef REMOTEQUEUEWIDGET_H
#define REMOTEQUEUEWIDGET_H

#include <QtGui/QWidget>

namespace Ui {
class RemoteQueueWidget;
}

namespace MoleQueue
{
class Client;
class QueueRemote;
class TemplateKeywordDialog;

/**
 * @class RemoteQueueWidget remotequeuewidget.h <molequeue/remotequeuewidget.h>
 *
 * @brief A generic configuration dialog for remote queuing systems.
 *
 * @author David C. Lonie
 */
class RemoteQueueWidget: public QWidget
{
  Q_OBJECT

public:
  explicit RemoteQueueWidget(QueueRemote *queue, QWidget *parentObject = 0);
  ~RemoteQueueWidget();

protected slots:
  void updateGuiFromQueue();

  void testConnection();

  void sleepTest();

  void updateSubmissionCommand(const QString &command);
  void updateKillCommand(const QString &command);
  void updateRequestQueueCommand(const QString &command);
  void updateLaunchScriptName(const QString &name);
  void updateWorkingDirectoryBase(const QString &dir);
  void updateDefaultMaxWallTime();
  void updateHostName(const QString &hostName);
  void updateUserName(const QString &userName);
  void updateSshPort(int sshPort);
  void showTemplateHelp();
  void updateLaunchTemplate();

private:
  Ui::RemoteQueueWidget *ui;
  QueueRemote *m_queue;
  Client *m_client; // Used for submitting test jobs.
  TemplateKeywordDialog *m_helpDialog;
};

} // end namespace MoleQueue

#endif // REMOTEQUEUEWIDGET_H
