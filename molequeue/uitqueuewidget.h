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

#ifndef UITQUEUEWIDGET_H
#define UITQUEUEWIDGET_H

#include "abstractqueuesettingswidget.h"
#include "queues/uit/userhostassoclist.h"

namespace Ui {
class UitQueueWidget;
}

namespace MoleQueue
{
class Client;
class QueueUit;
class TemplateKeywordDialog;

/**
 * @class UitQueueWidget uitqueuewidget.h <molequeue/remotequeuewidget.h>
 *
 * @brief A configuration dialog for UIT queuing systems.
 */
class UitQueueWidget: public AbstractQueueSettingsWidget
{
  Q_OBJECT

public:
  explicit UitQueueWidget(QueueUit *queue, QWidget *parentObject = 0);
  ~UitQueueWidget();

public slots:
  void save();
  void reset();

protected slots:
  void testConnection();
  void sleepTest();
  void showHelpDialog();
  void updateHostList(const Uit::UserHostAssocList &list);

private slots:
  //void showFileDialog();

private:
  Ui::UitQueueWidget *ui;
  QueueUit *m_queue;
  Client *m_client; // Used for submitting test jobs.
  TemplateKeywordDialog *m_helpDialog;
};

} // end namespace MoleQueue

#endif //UITQUEUEWIDGET_H
