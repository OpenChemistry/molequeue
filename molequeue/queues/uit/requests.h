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

#ifndef REQUESTS_H_
#define REQUESTS_H_

#include "job.h"
#include "jobsubmissioninfo.h"
#include "userhostassoclist.h"
#include "wsdl_uitapi.h"
#include "jobeventlist.h"
#include "dirlistinginfo.h"

#include <QtCore/QObject>


class KDSoapJob;

namespace MoleQueue {
namespace Uit {

class Session;

/**
 * @brief Abstract base class of all UIT SOAP requests.
 */
class Request : public QObject
{
  Q_OBJECT
public:

  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  Request(Session *session,
          QObject *parentObject = 0);

public slots:
  /**
   * Slot used to submit request to UIT server.
   */
  void submit();

  /**
   * @return the host ID of the UIT host this request is associated with.
   */
  qint64 hostId() const {
    return m_hostID;
  }
  /**
   * @param id The host ID.
   */
  void setHostId(qint64 id) {
    m_hostID = id;
  }

  /**
   * @return The user name this request is associated with.
   */
  QString userName() const {
    return m_userName;
  }
  /**
   * @param user The user name this request should be associated with.
   */
  void setUserName(const QString& user) {
    m_userName = user;
  }

signals:
  /**
   * Emitted when this request is completed.
   */
  void finished();
  /**
   * Emitted is an error occurs.
   *
   * @param errorString The error message.
   */
  void error(const QString &errorString);

protected:
  // The UIT session
  Session *m_session;
  // The SOAP response
  KDSoapMessage m_response;
  qint64 m_hostID;
  QString m_userName;

  /**
   * Overridden by subclasses to create appropriate KDSoapJob for this request.
   */
  virtual KDSoapJob *createJob() = 0;

protected slots:
  /**
   * Slot called by KDSoap when the SOAP request is complete.
   *
   * @param job The SOAP request that just completed.
   */
  void finished(KDSoapJob *job);

private:
  /**
   * @return true, if the fail was the result of a invalid token ( we need to
   * authenticate ), false otherwise.
   */
  bool isTokenError(const KDSoapMessage& fault);

  /**
   * Process a SOAP fault message.
   *
   * @param fault The fault message to process.
   */
  void processFault(const KDSoapMessage& fault);

  /**
   * Process the SOAP reply.
   *
   * @param reply The SOAP reply from the UIT server.
   */
  void processReply(const KDSoapMessage& reply);
};

/**
 * Concrete Request class for submitBatchScriptJob message.
 */
class SubmitBatchScriptJobRequest: public Request
{
  Q_OBJECT
public:
  SubmitBatchScriptJobRequest(Session *session,
                              QObject *parentObject = 0);

  /**
   * @return The Job being submitted.
   */
  Job job() const {
    return m_job;
  }
  void setJob(const Job &j) {
    m_job = j;
  }

  /**
   * @return The path the batch script to submit.
   */
  QString batchScript() const {
    return m_batchScript;
  }

  /**
   * @param script The path to the batch script to submit.
   */
  void setBatchScript(const QString& script) {
    m_batchScript = script;
  }

  /**
   * @return The working directory to submit the job from.
   */
  QString workingDir() {
    return m_workingDir;
  }

  /**
   * @param dir The working directory to submit the job from.
   */
  void setWorkingDir(const QString& dir) {
    m_workingDir = dir;
  }

  /**
   * @return The JobSubmissionInfo object representing the response from the
   * UIT server.
   *
   * Note: Will only produce a populated JobSubmissionInfo object after the
   * finished() signal has been emitted.
   */
  JobSubmissionInfo jobSubmissionInfo();

protected:
  KDSoapJob *createJob();

private:
  Job m_job;
  QString m_batchScript;
  QString m_workingDir;
};

/**
 * @brief Concrete Request class for getUserHostAssoc message.
 */
class GetUserHostAssocRequest: public Request
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  GetUserHostAssocRequest(Session *session,
                          QObject *parentObject = 0);


  /**
   * @return The UserHostAssocList object representing the response from the
   * UIT server.
   *
   * Note: Will only produce a populated UserHostAssocList object after the
   * finished() signal has been emitted.
   */
  UserHostAssocList userHostAssocList();

protected:
  KDSoapJob *createJob();
};

/**
 * @brief Concrete Request class for createDirectory message.
 */
class CreateDirectoryRequest: public Request
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  CreateDirectoryRequest(Session *session,
                        QObject *parentObject = 0);

  /**
   * @return The directory the request is going to create.
   */
  QString directory() const {
    return m_directory;
  }
  /**
   * @param dir The directory to create.
   */
  void setDirectory(const QString& dir) {
    m_directory = dir;
  }

  /**
   * @return The job associated with this request.
   */
  Job job() const{
    return m_job;
  }
  /**
   * @param j The job associated with this request.
   */
  void setJob(const Job& j) {
    m_job = j;
  }

protected:
  KDSoapJob *createJob();

private:
  Job m_job;
  QString m_directory;
};

/**
 *  @brief Concrete Request class for getStreamingFileUploadURL message.
 */
class GetStreamingFileUploadURLRequest: public Request
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  GetStreamingFileUploadURLRequest(Session *session,
                                   QObject *parentObject = 0);

  /**
   * @return The job associated with this request.
   */
  Job job() const {
    return m_job;
  }
  /**
   * @param j The job associated with this request.
   */
  void setJob(const Job& j) {
    m_job = j;
  }

  /**
   * @return The file upload URL returned by the server.
   *
   * Note: Will only produce valid URL after the
   * finished() signal has been emitted.
   *
   */
  QString url();

protected:
  KDSoapJob *createJob();

private:
  Job m_job;
};

/**
 * @brief Concrete Request class for getJobsForHostForUserByNumDays message.
 */
class GetJobsForHostForUserByNumDaysRequest: public Request
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  GetJobsForHostForUserByNumDaysRequest(Session *session,
                                   QObject *parentObject = 0);

  /**
   * @return The number of days of events to retrieve.
   */
  qint64 numDays() const {
    return m_numDays;
  }
  /**
   * @param days The number of days of events to retrieve.
   */
  void setNumDays(qint64 days) {
    m_numDays = days;
  }

  /**
   * @return The user to filter events by.
   */
  QString searchUser() const {
    return m_searchUser;
  }
  /**
   * @param user The user to filter events by.
   */
  void setSearchUser(const QString& user) {
    m_searchUser = user;
  }

  /**
   * @return The JobEventList object representing the response from the
   * UIT server.
   *
   * @param jobIds The list of job ID to filter the events on.
   *
   * Note: Will only produce a populated JobEventList object after the
   * finished() signal has been emitted.
   */
  JobEventList jobEventList(QList<qint64> jobIds);

protected:
  KDSoapJob *createJob();

private:
  QString m_searchUser;
  qint64 m_numDays;

};

/**
 * @brief Concrete Request class for getStreamingFileDownloadURL message.
 */
class GetStreamingFileDownloadURLRequest: public Request
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @parentObject The parent object.
   */
  GetStreamingFileDownloadURLRequest(Session *session,
                                     QObject *parentObject = 0);

  /**
   * @return The file download URL returned by the server.
   *
   * Note: Will only produce valid URL after the
   * finished() signal has been emitted.
   *
   */
  QString url();

protected:
  KDSoapJob *createJob();
};

/**
 * @brief Concrete Request class for getDirectoryListing message.
 */
class GetDirectoryListingRequest: public Request
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  GetDirectoryListingRequest(Session *session,
                             QObject *parentObject = 0);

  /**
   * @return The directory to get the list of.
   */
  QString directory() const {
    return m_directory;
  }
  /**
   * @param The directory we want a listing for.
   */
  void setDirectory(const QString& dir) {
    m_directory = dir;
  }

  /**
   * @return The DirListingInfo object representing the response from the
   * UIT server.
   *
   * @param jobIds The list of job ID to filter the events on.
   *
   * Note: Will only produce a populated DirListingInfo object after the
   * finished() signal has been emitted.
   */
  DirListingInfo dirListingInfo();


protected:
  KDSoapJob *createJob();

private:
  QString m_directory;
};

/**
 * @brief Concrete Request class for deleteFileRequest message.
 */
class DeleteFileRequest: public Request
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  DeleteFileRequest(Session *session,
                    QObject *parentObject = 0);

  /**
   * @return The file being deleted.
   */
  QString file() const {
    return m_file;
  }
  /**
   * @param f The file to be deleted.
   */
  void setFile(const QString& f) {
    m_file = f;
  }

protected:
  KDSoapJob *createJob();

private:
  QString m_file;
};

/**
 * @brief Concrete Request class for deleteDirectory message.
 */
class DeleteDirectoryRequest: public Request
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  DeleteDirectoryRequest(Session *session,
                         QObject *parentObject = 0);

  /**
   * @return The directory being deleted.
   */
  QString directory() const {
    return m_directory;
  }
  /**
   * @param dir The directory to be deleted.
   */
  void setDirectory(const QString& dir) {
    m_directory = dir;
  }

protected:
  KDSoapJob *createJob();

private:
  QString m_directory;
};

/**
 *  @brief Concrete Request class for cancelJob message.
 */
class CancelJobRequest: public Request
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  CancelJobRequest(Session *session,
                   QObject *parentObject = 0);

  /**
   * @return The job associated with this request.
   */
  Job job() const {
    return m_job;
  }
  /**
   * @param The job associated with this request.
   */
  void setJob(const Job& j) {
    m_job = j;
  }

protected:
  KDSoapJob *createJob();

private:
  Job m_job;
};

/**
 *  @brief Concrete Request class for statFile message.
 */
class StatFileRequest: public Request
{
  Q_OBJECT
public:
  /**
   * @param session The UIT session.
   * @param parentObject The parent object.
   */
  StatFileRequest(Session *session,
                   QObject *parentObject = 0);

  /**
   * @return The job associated with this request.
   */
  Job job() const {
    return m_job;
  }
  /**
   * @param The job associated with this request.
   */
  void setJob(const Job& j) {
    m_job = j;
  }

  QString filename() const {
    return m_filename;
  }
  void setFilename(QString name) {
    m_filename = name;
  }

  QString output();
  bool exists();

protected:
  KDSoapJob *createJob();

private:
  Job m_job;
  QString m_filename;
};


} /* namespace Uit */
} /* namespace MoleQueue */

#endif /* REQUESTS_H_ */
