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

#include "abstractqueuesettingswidget.h"

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
class RemoteQueueWidget: public AbstractQueueSettingsWidget
{
  Q_OBJECT

public:
  explicit RemoteQueueWidget(QueueRemote *queue, QWidget *parentObject = 0);
  ~RemoteQueueWidget();

public slots:
  void save();
  void reset();

protected slots:
  void testConnection();
  void sleepTest();
  void showHelpDialog();

private:
  Ui::RemoteQueueWidget *ui;
  QueueRemote *m_queue;
  Client *m_client; // Used for submitting test jobs.
  TemplateKeywordDialog *m_helpDialog;
};

} // end namespace MoleQueue

#endif // REMOTEQUEUEWIDGET_H
