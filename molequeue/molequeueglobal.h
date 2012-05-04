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

namespace MoleQueue
{

/// Type for various ids
typedef quint32 mqIdType;

/// Type for RPC packets
typedef QByteArray mqPacketType;

/**
  * Enumeration defining states that jobs are allowed to be in.
  */
enum JobState {
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
  * Enumeration defining possible job submission error codes.
  */
enum JobSubmissionErrorCode {
  /// No error occurred.
  Success = 0
};

} // end namespace MoleQueue

#endif // MOLEQUEUEGLOBAL_H
