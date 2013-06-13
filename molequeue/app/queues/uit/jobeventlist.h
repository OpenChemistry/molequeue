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

#ifndef JOBEVENTLIST_H_
#define JOBEVENTLIST_H_

#include "jobevent.h"

#include <QtCore/QList>

namespace MoleQueue {
namespace Uit {

/**
 * @brief class used to model a UIT JobEvent list.
 */
class JobEventList
{
public:
  JobEventList();
  /**
   * @param other The instance to copy.
   */
  JobEventList(const JobEventList &other);

  /**
   * @return The list of JobEvents.
   */
  QList<JobEvent> jobEvents() const
  {
    return m_jobEvents;
  }

  /**
   * @return true, if the instance represents a valid JobEventList document,
   * false otherwise.
   */
  bool isValid() const
  {
    return m_valid;
  }

  /**
   * @return The raw XML that this object was generated from.
   */
  QString xml() const
  {
    return m_xml;
  }

  /**
   * Converts JobEventList XML document into object model.
   *
   * @return The object model.
   * @param xml The XML document to parse.
   */
  static JobEventList fromXml(const QString &xml);

  /**
   * Converts JobEventList XML document into object model.
   *
   * @return The object model.
   * @param xml The XML document to parse.
   * @param userName The username to filter JobEvents on.
   * @param jobIds The job ids to filer JobEvents on.
   */
  static JobEventList fromXml(const QString &xml, const QString &userName,
                              QList<qint64> jobIds);

private:
  bool m_valid;
  QList<JobEvent> m_jobEvents;
  QString m_xml;

  void setContent(const QString &xml, const QString &userName,
                  QList<qint64> jobIds);
};

} /* namespace Uit */
} /* namespace MoleQueue */
#endif /* JOBEVENTLIST_H_ */
