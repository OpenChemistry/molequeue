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

#ifndef JOBEVENT_H_
#define JOBEVENT_H_

#include <QtCore/QString>

namespace MoleQueue {
namespace Uit {

/**
 * @brief class using to model UIT JobEvent.
 */
class JobEvent
{
public:
  JobEvent();
  /**
   * @param other The JobEvent instance being copied.
   */
  JobEvent(const JobEvent &other);
  /**
    * @param other The JobEvent instance being assigned.
    */
  JobEvent &operator=(const JobEvent &other);

  /**
   * @return The account host.
   */
  QString acctHost() const {
    return m_acctHost;
  }
  /**
   * @param host The account host.
   */
  void setAcctHost(const QString& host) {
    m_acctHost = host;
  }

  /**
   * @return The event time for this job event.
   */
  qint64 eventTime() const {
    return m_eventTime;
  }
  /**
   * @param time The event time.
   */
  void setEventTime(qint64 time) {
    m_eventTime = time;
  }

  /**
   * @return The event type for this job event
   */
  QString eventType() const {
    return m_eventType;
  }
  /**
   * @param type The event type.
   */
  void setEventType(const QString& type) {
    m_eventType = type;
  }

  /**
   * @return The job ID for this job event.
   */
  qint64 jobId() const {
    return m_jobID;
  }
  /**
   * @param id The job ID.
   */
  void setJobId(qint64 id) {
    m_jobID = id;
  }

  /**
   * @return The job queue associated with this job event.
   */
  QString jobQueue() const {
    return m_jobQueue;
  }
  /**
   * @param queue The queue name.
   */
  void setJobQueue(const QString& queue) {
    m_jobQueue = queue;
  }

  /**
   * @return The job status associated with this job event.
   */
  QString jobStatus() const {
    return m_jobStatus;
  }
  /**
   * @param status The job status.
   */
  void setJobStatus(const QString &status) {
    m_jobStatus = status;
  }

  /**
   * @return The job status text associated with this job event.
   */
  QString jobStatusText() const {
    return m_jobStatusText;
  }
  /**
   * @param text The job status text.
   */
  void setJobStatusText(const QString& text) {
    m_jobStatusText = text;
  }

private:
  QString m_acctHost;
  QString m_eventType;
  qint64 m_eventTime;
  qint64 m_jobID;
  QString m_jobQueue;
  QString m_jobStatus;
  QString m_jobStatusText;

};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* JOBEVENT_H_ */
