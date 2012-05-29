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

#include "remote.h"

#include "../job.h"
#include "../program.h"
#include "../terminalprocess.h"
#include "../sshcommand.h"

#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include <QtGui>

namespace MoleQueue {

QueueRemote::QueueRemote(QObject *parentObject) :
  Queue("Remote", parentObject)
{
}

QueueRemote::~QueueRemote()
{
}

QWidget* QueueRemote::settingsWidget() const
{
  QWidget *widget = new QWidget;
  QFormLayout *layout = new QFormLayout;
  layout->addRow("Host name: ", new QLineEdit(widget));
  layout->addRow("Port: ", new QLineEdit(widget));
  layout->addRow("User name: ", new QLineEdit(widget));
  layout->addRow("SSH Certificate", new QLineEdit(widget));
  layout->addRow("Local Directory: ", new QLineEdit(widget));
  layout->addRow("Remote Directory: ", new QLineEdit(widget));
  widget->setLayout(layout);
  return widget;
}

bool QueueRemote::submitJob(const Job *job)
{
  /// @todo This needs to be rewritten
  Q_UNUSED(job);
  /*
  m_jobs.push_back(job);
  job->setJobState(MoleQueue::RemoteQueued);
  emit(jobAdded(job));
  submitJob(m_jobs.size() - 1);
  */
  return true;
}

} // End namespace
