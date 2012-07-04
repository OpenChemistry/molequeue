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

#ifndef ADDQUEUEDIALOG_H
#define ADDQUEUEDIALOG_H

#include <QDialog>

namespace Ui {
    class AddQueueDialog;
}

namespace MoleQueue {

class QueueManager;

class AddQueueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddQueueDialog(QueueManager *queueManager, QWidget *parentObject = 0);
    ~AddQueueDialog();

private slots:
  void addQueue();

private:
    Ui::AddQueueDialog *ui;
    QueueManager *m_queueManager;
};

} // end MoleQueue namespace

#endif // ADDQUEUEDIALOG_H
