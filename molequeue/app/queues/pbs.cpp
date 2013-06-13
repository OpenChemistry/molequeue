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

#include "pbs.h"

#include "logger.h"

#include <QtCore/QDebug>

namespace MoleQueue
{

QueuePbs::QueuePbs(QueueManager *parentManager) :
  QueueRemoteSsh("Remote (PBS)", parentManager)
{
  m_submissionCommand = "qsub";
  m_killCommand = "qdel";
  m_requestQueueCommand = "qstat";
  m_launchScriptName = "job.pbs";

  m_launchTemplate =
      "#!/bin/sh\n"
      "#\n"
      "# Sample job script provided by MoleQueue.\n"
      "#\n"
      "#These commands set up the Grid Environment for your job:\n"
      "#PBS -N MoleQueueJob-$$moleQueueId$$\n"
      "#PBS -l procs=$$numberOfCores$$\n"
      "#PBS -l walltime=$$maxWallTime$$\n"
      "\n"
      "cd $PBS_O_WORKDIR\n"
      "$$programExecution$$\n";

  // qstat will return an exit code of 153 if a job has completed.
  m_allowedQueueRequestExitCodes.append(153);
  // unless it's an ezHPC fork. Then it will return 35.
  m_allowedQueueRequestExitCodes.append(35);
}

QueuePbs::~QueuePbs()
{
}

bool QueuePbs::parseQueueId(const QString &submissionOutput, IdType *queueId)
{
  // Assuming submissionOutput is:
  // <jobid>.<hostname>
  QRegExp parser ("^(\\d+)");

  int ind = parser.indexIn(submissionOutput);
  if (ind >= 0) {
    bool ok;
    *queueId = static_cast<IdType>(parser.cap(1).toInt(&ok));
    return ok;
  }
  return false;
}

bool QueuePbs::parseQueueLine(const QString &queueListOutput,
                              IdType *queueId, JobState *state)
{
  // Expecting qstat output is:
  // Job id           Name             User             Time Use S Queue
  // ---------------- ---------------- ---------------- -------- - -----
  //  4807             scatter          user01           12:56:34 R batch
  QRegExp parser ("^\\s*(\\d+)\\S*" // job-ID
                  "\\s+\\S+"    // name
                  "\\s+\\S+"    // user
                  "\\s+\\S+"    // time
                  "\\s+(\\w+)"); // state

  QString stateStr;
  int ind = parser.indexIn(queueListOutput);
  if (ind >= 0) {
    bool ok;
    *queueId = static_cast<IdType>(parser.cap(1).toInt(&ok));
    if (!ok)
      return false;
    stateStr = parser.cap(2).toLower();

    if (stateStr == "r" ||
        stateStr == "e" ||
        stateStr == "c") {
      *state = MoleQueue::RunningRemote;
      return true;
    }
    else if (stateStr == "q" ||
             stateStr == "h" ||
             stateStr == "t" ||
             stateStr == "w" ||
             stateStr == "s") {
      *state = MoleQueue::QueuedRemote;
      return true;
    }
    else {
      Logger::logWarning(tr("Unrecognized queue state '%1' in %2 queue '%3'. "
                            "Queue line:\n%4")
                         .arg(stateStr).arg(typeName()).arg(name())
                         .arg(queueListOutput));
      return false;
    }
  }
  return false;
}

} // End namespace
