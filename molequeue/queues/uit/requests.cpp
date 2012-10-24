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

#include "wsdl_uitapi.h"
#include "requests.h"
#include "session.h"

namespace MoleQueue {
namespace Uit {

Request::Request(Session *session,
                       QObject *parentObject)
  : QObject(parentObject), m_session(session), m_hostID(-1)
{

}

void Request::submit()
{
  KDSoapJob *job = createJob();

  connect(job, SIGNAL(finished(KDSoapJob *)),
          this, SLOT(finished(KDSoapJob *)));

  job->start();
}

void Request::finished(KDSoapJob *job)
{
  m_response = job->reply();

  // UIT error case
  if (job->isFault()) {
    processFault(m_response);
  // Process response to submit
  } else {
    emit finished();

  }
}

void Request::processFault(const KDSoapMessage &fault)
{
  if (isTokenError(fault)) {
    m_session->authenticate(this,
                            SLOT(submit()),
                            this,
                            SIGNAL(error(const QString)));
  }
  else {
    emit error(fault.faultAsString());
  }
}

bool Request::isTokenError(const KDSoapMessage& fault)
{
  return fault.arguments().child("faultstring").value().toString()
       == "java.lang.Exception: Invalid Token";
}

SubmitBatchScriptJobRequest::SubmitBatchScriptJobRequest(Session *session,
                                                         QObject *parentObject)
  : Request(session, parentObject)
{


}

KDSoapJob * SubmitBatchScriptJobRequest::createJob()
{
 SubmitBatchScriptJobJob *soapJob = new SubmitBatchScriptJobJob(
                                          m_session->uitService(), this);

 soapJob->setToken(m_session->token());
 soapJob->setHostID(m_hostID);
 soapJob->setBatchScript("job.uit");
 soapJob->setWorkingDir(m_workingDir);
 soapJob->setUsername(m_userName);

 return soapJob;
}

JobSubmissionInfo SubmitBatchScriptJobRequest::jobSubmissionInfo()
{
  QString responseXml = m_response.childValues()
                        .child(QLatin1String("submitBatchScriptJobReturn"))
                        .value().value<QString>();

  JobSubmissionInfo info = JobSubmissionInfo::fromXml(responseXml);

  return info;
}

GetUserHostAssocRequest::GetUserHostAssocRequest(Session *session,
                                                 QObject *parentObject)
  : Request(session, parentObject)
{

}

KDSoapJob * GetUserHostAssocRequest::createJob()
{
 GetUserHostAssocJob *soapJob = new GetUserHostAssocJob(
                                          m_session->uitService(), this);
 soapJob->setToken(m_session->token());

 return soapJob;
}

UserHostAssocList GetUserHostAssocRequest::userHostAssocList()
{
  QString responseXml = m_response.childValues()
                        .child(QLatin1String("getUserHostAssocReturn"))
                        .value().value<QString>();

  UserHostAssocList info = UserHostAssocList::fromXml(responseXml);

  return info;
}


CreateDirectoryRequest::CreateDirectoryRequest(Session *session,
                                               QObject *parentObject)
  : Request(session, parentObject)
{

}

KDSoapJob * CreateDirectoryRequest::createJob()
{
  CreateDirectoryJob *soapJob = new CreateDirectoryJob(
                                     m_session->uitService(), this);
  soapJob->setToken(m_session->token());
  soapJob->setHostID(m_hostID);
  soapJob->setUsername(m_userName);
  soapJob->setDirectory(m_directory);

 return soapJob;
}

GetStreamingFileUploadURLRequest::GetStreamingFileUploadURLRequest(
  Session *session, QObject *parentObject)
  : Request(session, parentObject)
{

}

KDSoapJob * GetStreamingFileUploadURLRequest::createJob()
{
  GetStreamingFileUploadURLJob *soapJob = new GetStreamingFileUploadURLJob(
                                     m_session->uitService(), this);
  soapJob->setToken(m_session->token());

  return soapJob;
}

QString GetStreamingFileUploadURLRequest::url()
{
  return m_response.childValues()
         .child(QLatin1String("getStreamingFileUploadURLReturn"))
         .value().value<QString>();
}


GetJobsForHostForUserByNumDaysRequest::GetJobsForHostForUserByNumDaysRequest(
  Session *session,
  QObject *parentObject)
  : Request(session, parentObject)
{

}

KDSoapJob * GetJobsForHostForUserByNumDaysRequest::createJob()
{
  GetJobsForHostForUserByNumDaysJob *soapJob
    = new GetJobsForHostForUserByNumDaysJob(m_session->uitService(), this);

  soapJob->setToken(m_session->token());
  soapJob->setHostID(m_hostID);
  soapJob->setSearchUser(m_searchUser);
  soapJob->setUsername(m_userName);
  soapJob->setNumDays(m_numDays);

  return soapJob;
}

JobEventList GetJobsForHostForUserByNumDaysRequest::jobEventList(
  QList<qint64> jobIds)
{
  QString responseXml
    = m_response.childValues()
      .child(QLatin1String("getJobsForHostForUserByNumDaysReturn")).value()
      .value<QString>();

  JobEventList list = JobEventList::fromXml(responseXml, m_searchUser, jobIds);

  return list;
}

GetStreamingFileDownloadURLRequest::GetStreamingFileDownloadURLRequest(
  Session *session, QObject *parentObject)
  : Request(session, parentObject)
{

}

KDSoapJob * GetStreamingFileDownloadURLRequest::createJob()
{
GetStreamingFileDownloadURLJob *soapJob = new GetStreamingFileDownloadURLJob(
                                     m_session->uitService(), this);
  soapJob->setToken(m_session->token());

  return soapJob;
}

QString GetStreamingFileDownloadURLRequest::url()
{
  return m_response.childValues()
         .child(QLatin1String("getStreamingFileDownloadURLReturn")).value()
         .value<QString>();
}


GetDirectoryListingRequest::GetDirectoryListingRequest(Session *session,
                                               QObject *parentObject)
  : Request(session, parentObject)
{

}

KDSoapJob * GetDirectoryListingRequest::createJob()
{
  GetDirectoryListingJob *soapJob = new GetDirectoryListingJob(
                                      m_session->uitService(), this);
  soapJob->setToken(m_session->token());
  soapJob->setHostID(m_hostID);
  soapJob->setUsername(m_userName);
  soapJob->setDirectory(m_directory);

  return soapJob;
}

DirListingInfo GetDirectoryListingRequest::dirListingInfo()
{
  QString responseXml = m_response.childValues()
                        .child(QLatin1String("getDirectoryListingReturn"))
                        .value().value<QString>();

  DirListingInfo info = DirListingInfo::fromXml(responseXml);

  return info;
}

DeleteFileRequest::DeleteFileRequest(Session *session,
                                     QObject *parentObject)
  : Request(session, parentObject)
{

}

KDSoapJob * DeleteFileRequest::createJob()
{
  DeleteFileJob *soapJob = new DeleteFileJob(
                                     m_session->uitService(), this);
  soapJob->setToken(m_session->token());
  soapJob->setHostID(m_hostID);
  soapJob->setUsername(m_userName);
  soapJob->setFile(m_file);

  return soapJob;
}

DeleteDirectoryRequest::DeleteDirectoryRequest(Session *session,
                                               QObject *parentObject)
  : Request(session, parentObject)
{

}

KDSoapJob * DeleteDirectoryRequest::createJob()
{
  DeleteDirectoryJob *soapJob = new DeleteDirectoryJob(
                                      m_session->uitService(), this);
  soapJob->setToken(m_session->token());
  soapJob->setHostID(m_hostID);
  soapJob->setUsername(m_userName);
  soapJob->setDirectory(m_directory);

  return soapJob;
}


CancelJobRequest::CancelJobRequest(Session *session,
                                   QObject *parentObject)
  : Request(session, parentObject)
{

}

KDSoapJob * CancelJobRequest::createJob()
{
  CancelJobJob *soapJob = new CancelJobJob(m_session->uitService(), this);
  soapJob->setToken(m_session->token());
  soapJob->setHostID(m_hostID);
  soapJob->setUsername(m_userName);
  soapJob->setJobID(QString::number(m_job.queueId()));

  return soapJob;
}

StatFileRequest::StatFileRequest(Session *session,
                                 QObject *parentObject)
  : Request(session, parentObject)
{

}

KDSoapJob * StatFileRequest::createJob()
{
  StatFileJob *soapJob = new StatFileJob(m_session->uitService(), this);

  soapJob->setToken(m_session->token());
  soapJob->setHostID(m_hostID);
  soapJob->setUsername(m_userName);
  soapJob->setFilename(m_filename);

  return soapJob;
}

QString StatFileRequest::output()
{
  QString statOutput = m_response.childValues()
                         .child(QLatin1String("statFileReturn"))
                         .value().value<QString>();

  return statOutput;
}

bool StatFileRequest::exists()
{
  return output().contains("No such file or directory");
}

} /* namespace Uit */
} /* namespace MoleQueue */
