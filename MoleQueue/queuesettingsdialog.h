/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef QUEUESETTINGSDIALOG_H
#define QUEUESETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
    class QueueSettingsDialog;
}

namespace MoleQueue {

class Queue;

class QueueSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QueueSettingsDialog(Queue *queue, QWidget *parent = 0);
    ~QueueSettingsDialog();

private:
    Ui::QueueSettingsDialog *ui;
    Queue *m_queue;
};

} // end MoleQueue namespace

#endif // QUEUESETTINGSDIALOG_H
