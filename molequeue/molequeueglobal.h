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

#ifndef MOLEQUEUEGLOBAL_H
#define MOLEQUEUEGLOBAL_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QtContainerFwd>

namespace MoleQueue
{

/// Type for various ids
typedef quint32 IdType;

/// Type for RPC packets
typedef QByteArray PacketType;

/// Type for list queue/program names. Key is queue, value is list of supported
/// programs
typedef QList<QPair<QString, QStringList> > QueueListType;

/**
  * Enumeration defining states that jobs are allowed to be in.
  */
enum JobState {
  /// Unknown status
  Unknown = -1,
  /// Initial state of job, should never be entered.
  None = 0,
  /// Job has been accepted by the server and is being prepared (Writing input files, etc).
  Accepted,
  /// Job is being queued locally, either waiting for local execution or remote submission.
  LocalQueued,
  /// Job has been submitted to a remote queuing system.
  Submitted,
  /// Job is pending execution on a remote queuing system.
  RemoteQueued,
  /// Job is running locally.
  RunningLocal,
  /// Job is running remotely.
  RunningRemote,
  /// Job has completed.
  Finished
};

/**
 * Convert a JobState value to a string.
 *
 * @param state JobState
 * @return C string
 */
inline const char * jobStateToString(JobState state)
{
  switch (state)
  {
  case None:
    return "None";
  case Accepted:
    return "Accepted";
  case LocalQueued:
    return "LocalQueued";
  case Submitted:
    return "Submitted";
  case RemoteQueued:
    return "RemoteQueued";
  case RunningLocal:
    return "RunningLocal";
  case RunningRemote:
    return "RunningRemote";
  case Finished:
    return "Finished";
  default:
  case Unknown:
    return "Unknown";
  }
}

/**
 * Convert a string to a JobState value.
 *
 * @param state JobState string
 * @return JobState
 */
inline JobState stringToJobState(const char *str)
{
  if (qstrcmp(str, "None") == 0)
    return None;
  else if (qstrcmp(str, "Accepted") == 0)
    return Accepted;
  else if (qstrcmp(str, "LocalQueued") == 0)
    return LocalQueued;
  else if (qstrcmp(str, "Submitted") == 0)
    return Submitted;
  else if (qstrcmp(str, "RemoteQueued") == 0)
    return RemoteQueued;
  else if (qstrcmp(str, "RunningLocal") == 0)
    return RunningLocal;
  else if (qstrcmp(str, "RunningRemote") == 0)
    return RunningRemote;
  else if (qstrcmp(str, "Finished") == 0)
    return Finished;
  else
    return Unknown;
}

/**
  * Enumeration defining possible job submission error codes.
  */
enum JobSubmissionErrorCode {
  /// No error occurred.
  Success = 0
};

} // end namespace MoleQueue

#endif // MOLEQUEUEGLOBAL_H
