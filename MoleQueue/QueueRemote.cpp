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

namespace MoleQueue {

QueueRemote::QueueRemote(QObject *parent) :
  Queue("Remote", parent)
{
  setupPrograms();
}

QueueRemote::~QueueRemote()
{
}

bool QueueRemote::submit(const Program &job)
{
  m_jobs.push_back(job);
  m_jobs.back().setStatus(Program::QUEUED);
  emit(jobAdded(&m_jobs.back()));
  return true;
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

} // End namespace
