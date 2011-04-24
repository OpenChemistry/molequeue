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

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <QtCore/QDebug>

namespace MoleQueue {

QueueLocal::QueueLocal(QObject *parent) :
  Queue("Local", parent), m_process(0)
{
  setupPrograms();
}

QueueLocal::~QueueLocal()
{
}

bool QueueLocal::submit(const Program &job)
{
  m_jobs.push_back(job);
  m_jobs.back().setStatus(Program::QUEUED);
  runProgram();
  return true;
}

void QueueLocal::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
  QByteArray result = m_process->readAllStandardError();
  qDebug() << "Program output:" << result;
  qDebug() << "Return code:" << m_process->exitCode();
}

void QueueLocal::setupPrograms()
{
  Program gamess;
  gamess.setName("GAMESS");
  gamess.setRunDirect(true);
  gamess.setReplacement("input", "myInput.inp");
  gamess.setReplacement("ncpus", "2");
  gamess.setRunTemplate("/home/marcus/build/gamess/rungms $$input$$ 2010 $$ncpus$$");
  gamess.setWorkingDirectory("/home/marcus/local/gamess");

  m_programs["GAMESS"] = gamess;
}

void QueueLocal::runProgram()
{
  if (!m_process) {
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(finished(int,QProcess::ExitStatus)));
  }

  Program &job = m_jobs.back();
  QFile input(job.inputFile());
  QFileInfo info(input);
  input.copy(job.workingDirectory() + "/" + info.baseName() + ".inp");
  qDebug() << "Moving file" << job.inputFile() << "->"
           << job.workingDirectory() + "/" + info.baseName() + ".inp";
  job.setReplacement("input", info.baseName());

  qDebug() << "Job:" << job.workingDirectory() << job.expandedRunTemplate();

  m_process->setWorkingDirectory(job.workingDirectory());
  m_process->setStandardOutputFile(job.workingDirectory() + "/" +
                                   info.baseName() + ".gamout");
  m_process->start(job.expandedRunTemplate());
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

} // End namespace
