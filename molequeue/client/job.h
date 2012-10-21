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

#ifndef MOLEQUEUE_JOB_H
#define MOLEQUEUE_JOB_H

#include "molequeueclientexport.h"

#include <qjsonobject.h>

#include <QtCore/QString>
#include <QtCore/QVariant>

namespace MoleQueue
{

/*!
 * \class JobObject job.h <molequeue/client/job.h>
 * \brief Simple client-side representation for a MoleQueue job.
 * \author Marcus D. Hanwell
 *
 * The Job class provides a simple interface to the client side representation
 * of a job to be submitted to MoleQueue. Any fields that are not set/present
 * will be omitted entirely, or set to default values by MoleQueue. The internal
 * representation of a job (and the transport used) is JSON.
 *
 * The Job class and data structure is very lightweight, and designed to be
 * easily copied, modified and passed around.
 */

class MOLEQUEUECLIENT_EXPORT JobObject
{
public:
  JobObject();
  ~JobObject();

  /*! Set the \p value of the specified \p key. */
  void setValue(const QString &key, const QVariant &value);

  /*! Get the value of the specified \p key. */
  QVariant value(const QString &key) const;

  /*! Set the job up using the supplied JSON object. This replaces all previous
   * settings that may have been applied. */
  void fromJson(const QJsonObject &jsonObject) { m_value = jsonObject; }

  /*! Get the JSON object with the current job settings in it. */
  QJsonObject json() const { return m_value; }

  /*! Set the input file for the job. */
  void setInputFile(const QString &fileName, const QString &contents);
  void setInputFile(const QString &path);

protected:
  QJsonObject m_value;

  /*!
   * Generate a filespec JSON object form the supplied file name and contents.
   */
  QJsonObject fileSpec(const QString &fileName, const QString &contents);

  /*!
   * Generate a filespec JSON object form the supplied file path (must exist).
   */
  QJsonObject fileSpec(const QString &path);
};

} // End namespace MoleQueue

#endif // MOLEQUEUE_JOB_H
