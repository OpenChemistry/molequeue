/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "oar.h"

#include "logger.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>

namespace MoleQueue
{

QueueOar::QueueOar(QueueManager *parentManager) :
  QueueRemoteSsh("Remote (OAR)", parentManager)
{
  m_submissionCommand = "oarsub -S";
  m_killCommand = "oardel";
  m_requestQueueCommand = "oarstat";
  m_launchScriptName = "job-oar.sh";

  m_launchTemplate =
    "#!/bin/sh\n"
    "#OAR -l core=$$numberOfCores$$,walltime=00:05:00\n"
    "#OAR -O /temp_dd/igrida-fs1/my_login/SCRATCH/fake_job.\%jobid\%.output\n"
    "#OAR -E /temp_dd/igrida-fs1/my_login/SCRATCH/fake_job.\%jobid\%.error\n"
    "set -xv\n"
    "\n"
    "echo\n"
    "echo OAR_WORKDIR : $OAR_WORKDIR\n"
    "echo\n"
    "echo \"cat \\$OAR_NODE_FILE :\"\n"
    "cat $OAR_NODE_FILE\n"
    "echo\n"
    "\n"
    "echo \"\n"
    "##########################################################################\n"
    "# Where will your run take place ?\n"
    "#\n"
    "# * It is NOT recommanded to run in $HOME/... (especially to write), \n"
    "#   but rather in /temp_dd/igrida-fs1/...\n"
    "#   Writing directly somewhere in $HOME/... will necessarily cause NFS problems at some time.\n"
    "#   Please respect this policy.\n"
    "#\n"
    "# * The program to run may be somewhere in your $HOME/... however\n"
    "#\n"
    "##########################################################################\n"
    "\"\n"
    "\n"
    "TMPDIR=$SCRATCHDIR/$OAR_JOB_ID\n"
    "mkdir -p $TMPDIR\n"
    "cd $TMPDIR\n"
    "\n"
    "echo \"pwd :\"\n"
    "pwd\n"
    "\n"
    "echo\n"
    "echo \"=============== RUN ===============\"\n"
    "\n"
    "#-- FAKE RUN EXECUTION\n"
    "echo \"Running ...\"\n"
    "$$programExecution$$\n"
    "\n"
    "echo \"Done\"\n"
    "echo \"===================================\"\n"
    "\n"
    "echo\n"
    "echo OK\n";
}

QueueOar::~QueueOar()
{
}

QString QueueOar::generateQueueRequestCommand()
{
  QList<IdType> queueIds(m_jobs.keys());
  QStringList queueIdStringList;
  foreach (IdType id, queueIds) {
    if (id != InvalidId)
      queueIdStringList << QString::number(id);
  }

  return QString("%1 %2").arg(m_requestQueueCommand)
      .arg(queueIdStringList.join(" -j "));
}

bool QueueOar::parseQueueId(const QString &submissionOutput, IdType *queueId)
{
  // Assuming submissionOutput is:
  // OAR_JOB_ID=8160394
  QRegExp parser("OAR_JOB_ID=(\\d+)");

  int ind = parser.indexIn(submissionOutput);
  if (ind >= 0) {
    bool ok;
    *queueId = static_cast<IdType>(parser.cap(1).toInt(&ok));
    return ok;
  }
  return false;
}

bool QueueOar::parseQueueLine(const QString &queueListOutput,
                              IdType *queueId, JobState *state)
{
  *queueId = InvalidId;
  *state = Unknown;

  // Job id    S User     Duration   System message
  // --------- - -------- ---------- ------------------------------------------------
  // 8160394   R kchoi       0:01:18 R=1,W=0:10:0,J=B (Karma=0.000)

  QRegExp parser("^\\s*(\\d+)"  // job-ID
                 "\\s+(\\w+)"   // state
                 "\\s+\\S+");   // user

  int ind = parser.indexIn(queueListOutput);
  if (ind >= 0) {
    bool ok;
    *queueId = static_cast<IdType>(parser.cap(1).toInt(&ok));
    if (!ok)
      return false;

    QString stateStr(parser.cap(2).toLower());

    // JOB STATE CODES
    //
    // Jobs typically pass through several states in the course of
    // their execution.  The typical states are PENDING, RUNNING,
    // SUSPENDED, COMPLETING, and COMPLETED.  An explanation of each
    // state follows.
    //
    // L   LAUNCHING       Job  has  been allocated resources, but are waiting
    //                     for them to become ready for use (e.g. booting).
    // E   ERROR           Job  terminated  with  non-zero  exit code or other
    //                     failure condition.
    // W   WAITING         Job is awaiting resource allocation.
    // R   RUNNING         Job currently has an allocation.
    // T   TERMINATED      Job terminated.
    // F   FINISHED        Job just finished.
    if (stateStr == "l") {
      *state = MoleQueue::Accepted;
      return true;
    }
    else if (stateStr == "e") {
      *state = MoleQueue::Error;
      return true;
    }
    else if (stateStr == "w") {
      *state = MoleQueue::Submitted;
      return true;
    }
    else if (stateStr == "r") {
      *state = MoleQueue::RunningRemote;
      return true;
    }
    else if (stateStr == "t" || stateStr == "f" ) {
      *state = MoleQueue::Finished;
      return true;
    }
    else {
      Logger::logWarning(tr("Unrecognized queue state '%1' in %2 queue '%3'. "
                            "Queue line:\n'%4'")
                         .arg(stateStr).arg(typeName()).arg(name())
                         .arg(queueListOutput));
      return false;
    }
  }
  return false;
}

} // End namespace
