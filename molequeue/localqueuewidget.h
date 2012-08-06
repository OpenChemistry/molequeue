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

#ifndef MOLEQUEUE_LOCALQUEUEWIDGET_H
#define MOLEQUEUE_LOCALQUEUEWIDGET_H

#include "abstractqueuesettingswidget.h"

namespace Ui {
class LocalQueueWidget;
}

namespace MoleQueue {
class QueueLocal;

/// @brief Configuration widget for local queues.
class LocalQueueWidget : public AbstractQueueSettingsWidget
{
  Q_OBJECT

public:
  LocalQueueWidget(QueueLocal *queue, QWidget *parent_ = 0);
  ~LocalQueueWidget();

public slots:
  void save();
  void reset();

private:
  Ui::LocalQueueWidget *ui;
  QueueLocal *m_queue;
};

} // namespace MoleQueue

#endif // MOLEQUEUE_LOCALQUEUEWIDGET_H
