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

#ifndef JOBSUBMISSIONINFO_H_
#define JOBSUBMISSIONINFO_H_

#include <QtCore/QString>

namespace MoleQueue {
namespace Uit {

/**
 * @brief class used to model the response message returned by a UIT server in
 * response to a job submission.
 */
class JobSubmissionInfo
{
public:
    JobSubmissionInfo();
    /**
     * @param other The instance to copy.
     */
    JobSubmissionInfo(const JobSubmissionInfo &other);
    /**
     * @param other The instance to assign.
     */
    JobSubmissionInfo &operator=(const JobSubmissionInfo &other);

    /**
     * @return true if this object represents a valid JobSubmissionInfo
     * document, false otherwise.
     */
    bool isValid() const;

    /**
     * @return the job number.
     */
    qint64 jobNumber() const;

    /**
     * @return the STDOUT produced by submitting the job.
     */
    QString stdout() const;

    /**
     * @return the STDERR produced by submitting the job.
     */
    QString stderr() const;

    /**
     * @param xml The XML to parse and populate the object model with.
     */
    void setContent(const QString &xml);

    /**
     * @return The raw XML parse and used to populare the object model with.
     */
    QString xml() const;

    /**
     * Convert a JobSubmissionInfo XML document into object model.
     *
     * @return The JobSubmissionInfo object representing the XML.
     * @param xml The JobSubmissionInfo XML document to parse.
     */
    static JobSubmissionInfo fromXml(const QString &xml);

private:
    bool m_valid;
    qint64 m_jobNumber;
    QString m_stdout;
    QString m_stderr;
    QString m_xml;
};

} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* JOBSUBMISSIONINFO_H_ */
