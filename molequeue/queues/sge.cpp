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

#include "sge.h"

#include <QtCore/QDebug>

namespace MoleQueue
{

QueueSge::QueueSge(QueueManager *parentManager) :
  QueueRemote("Remote (SGE)", parentManager)
{
  m_submissionCommand = "qsub";
  m_killCommand = "qdel";
  m_requestQueueCommand = "qstat";
  m_launchScriptName = "job.sge";

  m_launchTemplate =
      "#!/bin/sh\n"
      "#\n"
      "# Sample job script provided by MoleQueue.\n"
      "#\n"
      "# Use BASH as job shell:\n"
      "#$ -S /bin/bash\n"
      "\n"
      "$$programExecution$$\n";
}

QueueSge::~QueueSge()
{
}

bool QueueSge::parseQueueId(const QString &submissionOutput, IdType *queueId)
{
  // Assuming submissionOutput is:
  // your job <jobID> ('batchFileName') has been submitted
  QRegExp parser ("^[Yy]our job (\\d+)");

  int ind = parser.indexIn(submissionOutput);
  if (ind >= 0) {
    bool ok;
    *queueId = static_cast<IdType>(parser.cap(1).toInt(&ok));
    return ok;
  }
  return false;
}

QString QueueSge::generateQueueRequestCommand()
{
  return QString ("%1 -u %2").arg(m_requestQueueCommand).arg(m_userName);
}

bool QueueSge::parseQueueLine(const QString &queueListOutput,
                              IdType *queueId, JobState *state)
{
  // Expecting qstat output is:
  //
  //  job-ID   prior   name         user      state   submit/start at     queue      function
  //  231      0       hydra        craig     r       07/13/96            durin.q    MASTER
  //                                                  20:27:15
  //  232      0       compile      penny     r       07/13/96            durin.q    MASTER
  //                                                  20:30:40
  //  230      0       blackhole    don       r       07/13/96            dwain.q    MASTER
  //                                                  20:26:10
  //  233      0       mac          elaine    r       07/13/96            dwain.q    MASTER
  //                                                  20:30:40
  //  234      0       golf         shannon   r       07/13/96            dwain.q    MASTER
  //                                                  20:31:44
  //  236      5       word         elaine    qw      07/13/96
  //                                                  20:32:07
  //  235      0       andrun       penny     qw      07/13/96 20:31:43
  QRegExp parser ("^\\s*(\\d+)" // job-ID
                  "\\s+\\S+"    // prior
                  "\\s+\\S+"    // name
                  "\\s+\\S+"    // user
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
        stateStr == "d" || // mark deleted/errored jobs as running for now
        stateStr == "e") {
      *state = MoleQueue::RunningRemote;
      return true;
    }
    else if (stateStr == "qw"||
             stateStr == "q" ||
             stateStr == "w" ||
             stateStr == "s" ||
             stateStr == "h" ||
             stateStr == "t") {
      *state = MoleQueue::RemoteQueued;
      return true;
    }
    else {
      qWarning() << Q_FUNC_INFO << "unrecognized queue state:" << stateStr
                 << "\n" << queueListOutput;
      return false;
    }
  }
  return false;
}

} // End namespace
