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
class QueueRemote;

class RemoteQueueWidget: public QWidget
{
  Q_OBJECT

public:
  explicit RemoteQueueWidget(QueueRemote *queue, QWidget *parentObject = 0);
  ~RemoteQueueWidget();

protected slots:
  void updateGuiFromQueue();

  void updateSubmissionCommand(const QString &command);
  void updateRequestQueueCommand(const QString &command);
  void updateLaunchScriptName(const QString &name);
  void updateWorkingDirectoryBase(const QString &dir);
  void updateHostName(const QString &hostName);
  void updateUserName(const QString &userName);
  void updateSshPort(int sshPort);
  void updateLaunchTemplate();

private:
  Ui::RemoteQueueWidget *ui;
  QueueRemote *m_queue;
};

} // end namespace MoleQueue

#endif // REMOTEQUEUEWIDGET_H
