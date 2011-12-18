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

#include "QueueSGE.h"

#include "job.h"
#include "program.h"
#include "terminalprocess.h"
#include "sshcommand.h"

#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

namespace MoleQueue {

QueueSGE::QueueSGE(QObject *parent) :
  Queue("Remote (SGE)", parent), m_ssh(0), m_timer(0), m_interval(60)
{
  setupPrograms();
  setupProcess();
  m_localDir = QDir::homePath() + "/local/SGE";
}

QueueSGE::~QueueSGE()
{
  delete m_ssh;
  if (m_timer)
    m_timer->deleteLater();
}

bool QueueSGE::submit(Job *job)
{
  m_jobs.push_back(job);
  job->setStatus(Job::QUEUED);
  emit(jobAdded(job));
  submitJob(m_jobs.size() - 1);
  return true;
}

void QueueSGE::jobStarted(Job *job)
{

}

void QueueSGE::jobFinished(Job *job)
{
  job->setStatus(Job::COMPLETE);
  emit(jobStateChanged(job));
  // Now retrieve our output files, and put them in the local queue store
  int i = 0;
  for (i = 0; i < m_jobs.size(); ++i)
    if (m_jobs[i] == job)
      break;
  QString localDir = m_localDir + "/" + QString::number(i + m_jobIndexOffset);
  m_ssh->copyDirFrom(job->workingDirectory(), localDir);
}

void QueueSGE::pollRemote()
{
  qDebug() << "Polling the remote system...";
  QString output;
  int exitCode;
  QString command = "source /etc/profile && qstat";
  m_ssh->execute(command, output, exitCode);
  qDebug() << "Poll:" << output << "exit:" << exitCode;
  QStringList lines = output.split("\n", QString::SkipEmptyParts);
  // Copy the map of active jobs, update the status of each. If any are left,
  // they probably finished - check they did and retrieve the results file(s).
  QMap<QString, Job *> jobs = m_remoteJobs;
  foreach(const QString &line, lines) {
    if (line.contains("job-ID") || line.contains("-----------"))
      continue;
    QString jobId = line.mid(0, 7).trimmed();
    QString state = line.mid(40, 5).trimmed();
    Job::Status status = Job::UNDEFINED;
    if (state == "qw")
      status = Job::REMOTEQUEUED;
    else if (state == "r")
      status = Job::RUNNING;
    if (jobs.contains(jobId)) {
      // Found the job, update its status and remove it from our temporary map
      if (jobs[jobId]->status() != status) {
        jobs[jobId]->setStatus(status);
        emit(jobStateChanged(jobs[jobId]));
      }
      jobs.remove(jobId);
    }
    qDebug() << "Jobs map length:" << jobs.size();
    qDebug() << "JobId:" << jobId << "\tState:" << state;
  }
  if (jobs.size() > 0) {
    // Assume for now that the job completed, mark it as such and attempt to
    // retrieve output files.
    foreach(const QString &key, jobs.keys()) {
      m_remoteJobs.remove(key);
      jobFinished(jobs[key]);
    }
  }
  if (m_remoteJobs.size() == 0) // No need to poll the remote queue anymore
    m_timer->stop();
}

void QueueSGE::setupPrograms()
{
  Program *gamess = new Program;
  gamess->setName("GAMESS");
  gamess->setRunDirect(true);
//  gamess->setReplacement("input", "myInput.inp");
//  gamess->setReplacement("ncpus", "2");
  gamess->setRunTemplate("/usr/local/bin/gms_sge.sh $$input$$.inp $$workingDirectory$$");
//  gamess->setWorkingDirectory("/nfs/Users/mhanwell/tests/gamess");
  gamess->setQueue(this);
  m_programs["GAMESS"] = gamess;

  Program *sleep = new Program;
  sleep->setName("sleep");
  sleep->setRunDirect(true);
//  sleep->setReplacement("time", "10");
  sleep->setRunTemplate("sleep $$time$$");
//  sleep->setWorkingDirectory("/home/marcus/local");
  sleep->setQueue(this);
  m_programs["sleep"] = sleep;
}

void QueueSGE::setupProcess()
{
  m_ssh = new SshCommand(this);
  m_ssh->setUserName("marcus.hanwell");
  m_ssh->setHostName("big.cluster.address");
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(pollRemote()));
}

void QueueSGE::submitJob(int jobId)
{
  Job *job = m_jobs[jobId];

  qDebug() << "Job (R):" << jobId << job->workingDirectory()
           << job->expandedRunTemplate();

  // This is the remote working directory
  job->setWorkingDirectory(job->workingDirectory() + "/"
                          + QString::number(jobId + m_jobIndexOffset));

  QString localDir = m_localDir + "/" + QString::number(jobId + m_jobIndexOffset);
  if (!job->input().isEmpty()) {
    QDir dir;
    dir.mkpath(localDir);
    QFile inputFile(localDir + "/" + job->inputFile());
    inputFile.open(QFile::WriteOnly);
    inputFile.write(job->input().toLocal8Bit());
    inputFile.close();
  }
  else {
    QFile input(job->inputFile());
    QFileInfo info(input);
    if (info.exists()) {
      input.copy(localDir + "/" + info.baseName() + ".inp");
      qDebug() << "Moving file" << job->inputFile() << "->"
               << localDir + "/" + info.baseName() + ".inp";
    }
    else {
      qDebug() << "Error - file not found.";
    }
  }

  QString command = "source /etc/profile && qsub -N \"" + job->title() + "\" "
      + job->expandedRunTemplate();
  qDebug() << "Running command:" << command;

  QString output;
  int exitCode;

  if (!job->inputFile().isEmpty()) {
    qDebug() << "Input file:" << job->inputFile();
    m_ssh->execute("mkdir -p " + job->workingDirectory(), output, exitCode);
    qDebug() << "mkdir:" << output << exitCode;
    m_ssh->copyTo(localDir + "/" + job->inputFile(),
                  job->workingDirectory());
  }
  else {
    qDebug() << "No input file.";
  }
  m_ssh->execute(command, output, exitCode);
  QStringList parts = output.split(" ", QString::SkipEmptyParts);
  // Check if the job was submitted successfully
  if (parts.size() > 3 && parts[0] == "Your" && parts[1] == "job") {
    job->setTitle(job->title() + " (jobId: " + parts[2] + ")");
    job->setStatus(Job::REMOTEQUEUED);
    m_remoteJobs[parts[2]] = job;
    if (!m_timer->isActive())
      m_timer->start(m_interval * 1000);
  }
  else
    job->setStatus(Job::FAILED);
  emit(jobStateChanged(job));
  qDebug() << "Run gamess:" << output << exitCode;
}

} // End namespace
