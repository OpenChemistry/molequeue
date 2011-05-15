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

#include "QueueLocal.h"

#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QVariant>
#include <QtCore/QSettings>
#include <QtCore/QThread>

#include <QtCore/QDebug>

namespace MoleQueue {

QueueLocal::QueueLocal(QObject *parent) :
  Queue("Local", parent), m_process(0), m_currentJob(0), m_cores(-1)
{
  setupPrograms();
}

QueueLocal::~QueueLocal()
{
  delete m_process;
}

void QueueLocal::readSettings(const QSettings &settings)
{
  Queue::readSettings(settings);
  m_cores = settings.value("cores", -1).toInt();
}

void QueueLocal::writeSettings(QSettings &settings) const
{
  Queue::writeSettings(settings);
  settings.setValue("cores", m_cores);
}

bool QueueLocal::submit(const Program &job)
{
  m_jobs.push_back(job);
  m_jobs.back().setStatus(Program::QUEUED);
  emit(jobAdded(&m_jobs.back()));
  if (m_currentJob == m_jobs.size() - 1)
    runProgram(m_jobs.size() - 1);
  return true;
}

void QueueLocal::jobStarted()
{
  QObject *sender = QObject::sender();
  if (sender) {
    qDebug() << "The job was successfully started:" << sender->property("JOB_ID");
    int id = sender->property("JOB_ID").toInt();
    m_jobs[id].setStatus(Program::RUNNING);
    emit(jobStateChanged(0));
  }
}

void QueueLocal::jobFinished()
{
  QObject *sender = QObject::sender();
  if (sender) {
    qDebug() << "The job was successfully finished:" << sender->property("JOB_ID");
    int id = sender->property("JOB_ID").toInt();
    m_jobs[id].setStatus(Program::COMPLETE);
    emit(jobStateChanged(0));
    // Submit the next job if there is one
    ++m_currentJob;
    if (m_currentJob < m_jobs.size())
      runProgram(m_currentJob);
  }
}

void QueueLocal::jobFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  QByteArray result = m_process->readAllStandardError();
  qDebug() << "Program output:" << result;
  qDebug() << "Return code:" << exitCode << exitStatus;

  QObject *sender = QObject::sender();
  if (!sender)
    return;

  qDebug() << "The job was successfully finished:" << sender->property("JOB_ID");
  int id = sender->property("JOB_ID").toInt();
  m_jobs[id].setStatus(Program::COMPLETE);
  emit(jobStateChanged(0));
  // Submit the next job if there is one
  ++m_currentJob;
  if (m_currentJob < m_jobs.size())
    runProgram(m_currentJob);
}

void QueueLocal::processStateChanged(QProcess::ProcessState newState)
{
  qDebug() << "Process state changed:" << newState;
}

void QueueLocal::setupPrograms()
{
  Program gamess;
  gamess.setName("GAMESS");
  gamess.setRunDirect(true);
  gamess.setReplacement("input", "myInput.inp");
  gamess.setReplacement("ncpus", QString::number(cores()));
  gamess.setRunTemplate("/home/marcus/build/gamess/rungms $$input$$ 2010 $$ncpus$$");
  gamess.setWorkingDirectory("/home/marcus/local/gamess");
  gamess.setQueue(this);
  m_programs["GAMESS"] = gamess;

  Program sleep;
  sleep.setName("sleep");
  sleep.setRunDirect(true);
  sleep.setReplacement("time", "10");
  sleep.setRunTemplate("sleep $$time$$");
  sleep.setWorkingDirectory("/home/marcus/local");
  sleep.setQueue(this);
  m_programs["sleep"] = sleep;
}

void QueueLocal::runProgram(int jobId)
{
  if (!m_process) {
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(started()), this, SLOT(jobStarted()));
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(jobFinished(int,QProcess::ExitStatus)));
    connect(m_process, SIGNAL(stateChanged(QProcess::ProcessState)),
            this, SLOT(processStateChanged(QProcess::ProcessState)));
  }

  Program &job = m_jobs[jobId];
  job.setWorkingDirectory(job.workingDirectory() + "/"
                          + QString::number(jobId + m_jobIndexOffset));
  QDir dir;
  dir.mkpath(job.workingDirectory());
  if (!job.input().isEmpty()) {
    QFile inputFile(job.workingDirectory() + "/" + job.inputFile());
    inputFile.open(QFile::WriteOnly);
    inputFile.write(job.input().toLocal8Bit());
    inputFile.close();
  }
  else {
    QFile input(job.inputFile());
    QFileInfo info(input);
    if (info.exists()) {
      input.copy(job.workingDirectory() + "/" + info.baseName() + ".inp");
      qDebug() << "Moving file" << job.inputFile() << "->"
               << job.workingDirectory() + "/" + info.baseName() + ".inp";
    }
    else {
      qDebug() << "Error - file not found.";
    }
  }
  QFileInfo info(job.workingDirectory() + "/" + job.inputFile());
  job.setReplacement("input", info.baseName());
  m_process->setProperty("JOB_ID", jobId);

  qDebug() << "Job:" << jobId << job.workingDirectory()
           << job.expandedRunTemplate();

  m_process->setWorkingDirectory(job.workingDirectory());
  if (job.name() == "GAMESS") {
    m_process->setStandardOutputFile(job.workingDirectory() + "/" +
                                     info.baseName() + ".gamout");
    job.setOutputFile(job.workingDirectory() + "/" +
                      info.baseName() + ".gamout");
  }

  m_process->start(job.expandedRunTemplate());

//  if (!m_process->waitForStarted()) {
//      qDebug() << "Failed to start GAMESS..." << m_process->errorString();
//      return;
//    }
  /*if (!m_process->waitForStarted()) {
    qDebug() << "Failed to start GAMESS...";
    return;
  }

  m_process->closeWriteChannel();
  if (!m_process->waitForFinished()) {
    m_process->close();
    qDebug() << "Failed to exit.";
  }
  QByteArray result = m_process->readAll();
  qDebug() << "scp output:" << result << "Return code:" << m_process->exitCode();
*/
}

int QueueLocal::cores() const
{
  if (m_cores > 0)
    return m_cores;
  else
    return QThread::idealThreadCount();
}

} // End namespace
