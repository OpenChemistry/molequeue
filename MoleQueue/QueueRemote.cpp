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

#include "QueueRemote.h"

#include "program.h"
#include "terminalprocess.h"
#include "sshcommand.h"

#include <QtCore/QDebug>

namespace MoleQueue {

QueueRemote::QueueRemote(QObject *parent) :
  Queue("Remote", parent), m_process(0)
{
  setupPrograms();
  setupProcess();
}

QueueRemote::~QueueRemote()
{
}

bool QueueRemote::submit(const Program &job)
{
  m_jobs.push_back(job);
  m_jobs.back().setStatus(Program::QUEUED);
  emit(jobAdded(&m_jobs.back()));
  submitJob(m_jobs.size() - 1);
  return true;
}

void QueueRemote::jobStarted()
{

}

void QueueRemote::jobFinished()
{

}

void QueueRemote::setupPrograms()
{
  Program gamess;
  gamess.setName("GAMESS");
  gamess.setRunDirect(true);
  gamess.setReplacement("input", "myInput.inp");
  gamess.setReplacement("ncpus", "2");
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

void QueueRemote::setupProcess()
{
  m_ssh = new SshCommand(this);
  m_ssh->setHostName("localhost");
}

void QueueRemote::submitJob(int jobId)
{
  Program &job = m_jobs[jobId];

  qDebug() << "Job (R):" << jobId << job.workingDirectory()
           << job.expandedRunTemplate();

  QString command = "ssh localhost " + job.expandedRunTemplate();
  qDebug() << "Running command:" << command;

  QString output;
  int exitCode;

  if (!job.inputFile().isEmpty()) {
    qDebug() << "Input file:" << job.inputFile();
    job.setWorkingDirectory(job.workingDirectory() + "/" + QString::number(jobId));
    m_ssh->execute("mkdir -p " + job.workingDirectory(), output, exitCode);
    m_ssh->copyTo(job.inputFile(), job.workingDirectory());
  }
  else {
    qDebug() << "No input file.";
  }
  m_ssh->execute(job.expandedRunTemplate(), output, exitCode);
  qDebug() << "Output:" << output << exitCode;
}

} // End namespace
