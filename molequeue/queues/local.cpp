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

#include "local.h"

#include "../job.h"

#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QVariant>
#include <QtCore/QSettings>
#include <QtCore/QThread>

#include <QtGui/QWidget>
#include <QtGui/QFormLayout>
#include <QtGui/QSpinBox>

#include <QtCore/QDebug>

namespace MoleQueue {

QueueLocal::QueueLocal(QObject *parentObject) :
  Queue("Local", parentObject), m_cores(-1)
{
}

QueueLocal::~QueueLocal()
{
}

void QueueLocal::readSettings(const QSettings &settings)
{
  Queue::readSettings(settings);
  m_cores = settings.value("cores", -1).toInt();

  // use all cores if no value set
  if(m_cores == -1){
    m_cores = QThread::idealThreadCount();
  }
}

void QueueLocal::writeSettings(QSettings &settings) const
{
  Queue::writeSettings(settings);
  settings.setValue("cores", m_cores);
}

QWidget* QueueLocal::settingsWidget() const
{
  QWidget *widget = new QWidget;

  QFormLayout *layout = new QFormLayout;
  QSpinBox *coresSpinBox = new QSpinBox(widget);
  coresSpinBox->setValue(m_cores);
  coresSpinBox->setMinimum(0);
  layout->addRow("Number of Cores: ", coresSpinBox);
  widget->setLayout(layout);

  return widget;
}

bool QueueLocal::submitJob(const Job *job)
{
  /// @todo This needs to be rewritten
  Q_UNUSED(job);
  /*
  m_jobs.push_back(job);
  job->setJobState(MoleQueue::Accepted);
  emit(jobAdded(job));
  if (m_currentJob == m_jobs.size() - 1)
    runProgram(m_jobs.size() - 1);
  */
  return true;
}

int QueueLocal::cores() const
{
  if (m_cores > 0)
    return m_cores;
  else
    return QThread::idealThreadCount() > 8 ? 8 : QThread::idealThreadCount();
}

} // End namespace
