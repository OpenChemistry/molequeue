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

#include "queueuit.h"

#include "uit/kerberoscredentials.h"
#include "uit/sslsetup.h"
#include "uit/authenticator.h"
#include "uit/userhostassoclist.h"
#include "uit/sessionmanager.h"
#include "uit/requests.h"
#include "uit/jobsubmissioninfo.h"
#include "uit/directoryupload.h"
#include "uit/directorydownload.h"
#include "uit/directorydelete.h"
#include "uit/directorycreate.h"

#include "job.h"
#include "jobmanager.h"
#include "logentry.h"
#include "logger.h"
#include "program.h"
#include "uitqueuewidget.h"
#include "server.h"
#include "credentialsdialog.h"
#include "logger.h"
#include "mainwindow.h"
#include "filesystemtools.h"
#include <qjsonobject.h>
#include <qjsondocument.h>

#include <QtCore/QTimer>
#include <QtCore/QXmlStreamWriter>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtXmlPatterns/QAbstractMessageHandler>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

namespace MoleQueue
{

const QString QueueUit::clientId = "0adc5b59-5827-4331-a544-5ba7922ec2b8";

QueueUit::QueueUit(QueueManager *parentObject)
  : QueueRemote("ezHPC UIT", parentObject), m_uitSession(NULL),
    m_kerberosRealm("HPCMP.HPC.MIL"), m_hostID(-1),  m_dialogParent(NULL),
    m_isCheckingQueue(false)
{
  setLaunchScriptName("job.uit");

  // ensure SSL certificates are loaded
  Uit::SslSetup::init();

  m_launchTemplate =
      "#!/bin/sh\n"
      "#\n"
      "# Sample job script provided by MoleQueue.\n"
      "#PBS -l procs=1\n"
      "#PBS -l walltime=01:00:00\n"
      "#PBS -A <replace> \n"
      "#PBS -q debug\n"
      "#\n"
      "$$programExecution$$\n";
}

QueueUit::~QueueUit()
{
}


bool QueueUit::writeJsonSettings(QJsonObject &json, bool exportOnly,
                                 bool includePrograms) const
{
  if (!QueueRemote::writeJsonSettings(json, exportOnly, includePrograms))
    return false;

  json["kerberosUserName"] = m_kerberosUserName;
  json["kerberosRealm"] = m_kerberosRealm;
  json["hostName"] = m_hostName;
  json["hostID"] = QString::number(m_hostID);

  return true;
}

bool QueueUit::readJsonSettings(const QJsonObject &json, bool importOnly,
                                bool includePrograms)
{
  // Validate JSON:
  if (!json["kerberosUserName"].isString() ||
      !json["kerberosRealm"].isString() ||
      !json["hostName"].isString() ||
      !json["hostID"].isString()) {
    Logger::logError(tr("Error reading queue settings: Invalid format:\n%1")
                     .arg(QJsonDocument(json).toJson().constData()));
    return false;
  }

  if (!QueueRemote::readJsonSettings(json, importOnly, includePrograms))
    return false;

  m_kerberosUserName = json["kerberosUserName"].toString();
  m_kerberosRealm = json["kerberosRealm"].toString();
  m_hostName = json["hostName"].toString();
  m_hostID = json["hostID"].toString().toLongLong();

  return true;
}

bool QueueUit::testConnection(QWidget *parentObject)
{
  uitSession()->authenticate(this, SLOT(testConnectionComplete(const QString&)),
                             this,
                             SLOT(testConnectionError(const QString&)));

  m_dialogParent = parentObject;

  return true;
}

void QueueUit::testConnectionComplete(const QString &token)
{
  QMessageBox::information(m_dialogParent, tr("Success"),
                           tr("Connection to UIT succeeded!"));

  Q_UNUSED(token);
}

void QueueUit::testConnectionError(const QString &errorMessage)
{
  QMessageBox::critical(m_dialogParent, tr("UIT Error"), errorMessage);
}

AbstractQueueSettingsWidget* QueueUit::settingsWidget()
{
  UitQueueWidget *widget = new UitQueueWidget (this);
  return widget;
}

void QueueUit::createRemoteDirectory(Job job)
{

  QString remoteDir = QString("%1/%2").arg(m_workingDirectoryBase)
                        .arg(job.moleQueueId());

  Uit::DirectoryCreate *create = new Uit::DirectoryCreate(uitSession(),
                                                          this);
  create->setHostId(m_hostID);
  create->setUserName(m_kerberosUserName);
  create->setJob(job);
  create->setDirectory(remoteDir);

  connect(create, SIGNAL(finished()),
          this, SLOT(remoteDirectoryCreated()));
  connect(create, SIGNAL(error(const QString &)),
          this, SLOT(createRemoteDirectoryError(const QString&)));

  create->start();
}

void QueueUit::createRemoteDirectoryError(const QString &errorString)
{
  Uit::DirectoryCreate *createRequest
    = qobject_cast<Uit::DirectoryCreate*>(sender());

  if (!createRequest) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not UitDirectoryCreate!"));
      return;
  }
  createRequest->deleteLater();

  Job job = createRequest->job();

  Logger::logWarning(tr("Cannot create remote directory %1.\n"
                          "%2")
                     .arg(createRequest->directory()).arg(errorString),
                     job.moleQueueId());
  // Retry submission:
  if (addJobFailure(job.moleQueueId()))
    m_pendingSubmission.append(job.moleQueueId());

  job.setJobState(MoleQueue::Error);

  emit uitMethodError(errorString);
}

void QueueUit::remoteDirectoryCreated()
{
  Uit::DirectoryCreate *createRequest
    = qobject_cast<Uit::DirectoryCreate*>(sender());

  if (!createRequest) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not UitDirectoryCreate!"));
      return;
  }
  createRequest->deleteLater();

  uploadInputFilesToHost(createRequest->job());
}

void QueueUit::copyInputFilesToHost(Job job)
{
  QString localDir = job.localWorkingDirectory();
  QString remoteDir = QDir::cleanPath((m_workingDirectoryBase));

  Uit::StatFileRequest *request = new Uit::StatFileRequest(uitSession(), this);
  request->setJob(job);
  request->setHostId(m_hostID);
  request->setUserName(m_kerberosUserName);
  request->setFilename(remoteDir);

  connect(request, SIGNAL(finished()),
          this, SLOT(processStatFileRequest()));
  connect(request, SIGNAL(error(const QString &)),
          this, SLOT(copyInputFilesToHostError(const QString &)));

  request->submit();
}

void QueueUit::processStatFileRequest()
{
  Uit::StatFileRequest *request = qobject_cast<Uit::StatFileRequest*>(sender());
  if (!request) {
     Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                      .arg("Sender is not StatFileRequest!"));
       return;
  }
  request->deleteLater();

  Job job = request->job();

  uploadInputFilesToHost(job);
}


void QueueUit::uploadInputFilesToHost(Job job)
{
  QString localDir = job.localWorkingDirectory();
  QString remoteDir = QDir::cleanPath(QString("%1/%2")
                                      .arg(m_workingDirectoryBase)
                                      .arg(job.moleQueueId()));

  Uit::DirectoryUpload *uploader = new Uit::DirectoryUpload(uitSession(), this);
  uploader->setHostId(m_hostID);
  uploader->setUserName(m_kerberosUserName);
  uploader->setLocalPath(localDir);
  uploader->setRemotePath(remoteDir);
  uploader->setJob(job);

  connect(uploader, SIGNAL(finished()),
          this, SLOT(inputFilesCopied()));
  connect(uploader, SIGNAL(error(const QString &)),
          this, SLOT(copyInputFilesToHostError(const QString &)));

  uploader->start();
}

void QueueUit::copyInputFilesToHostError(const QString &errorString)
{
  Uit::StatFileRequest *request
    = qobject_cast<Uit::StatFileRequest*>(sender());

  if (!request) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not UitDirUploader!"));
      return;
  }
  request->deleteLater();

  Job job = request->job();

  if (errorString.contains(Uit::FileSystemOperation::noSuchFileOrDir))
    createRemoteDirectory(job);
  else {
    Logger::logError(tr("UIT error copying input files: '%1'").arg(errorString),
                     job.moleQueueId());

    if (addJobFailure(job.moleQueueId()))
      m_pendingSubmission.append(job.moleQueueId());

    job.setJobState(MoleQueue::Error);

    emit uitMethodError(errorString);
  }
}

void QueueUit::inputFilesCopied()
{
  Uit::DirectoryUpload *uploader
    = qobject_cast<Uit::DirectoryUpload*>(sender());

  if (!uploader) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not UitDirUploader!"));
      return;
  }
  uploader->deleteLater();

  Job job = uploader->job();
  submitJobToRemoteQueue(job);
}

void QueueUit::submitJobToRemoteQueue(Job job)
{
  Uit::SubmitBatchScriptJobRequest *request
    = new Uit::SubmitBatchScriptJobRequest(uitSession(), this);

  const Program *prog = lookupProgram(job.program());

  // TODO Should be pust to queue??
  QString launchString = prog->launchTemplate();
  replaceKeywords(launchString, job);
  QString workingDir = QString("%1/%2").arg(m_workingDirectoryBase)
                                        .arg(job.moleQueueId());
  request->setHostId(hostId());
  request->setUserName(m_kerberosUserName);
  request->setJob(job);
  request->setBatchScript(launchString);
  request->setWorkingDir(workingDir);

  connect(request, SIGNAL(finished()),
          this, SLOT(jobSubmittedToRemoteQueue()));
  connect(request, SIGNAL(error(const QString&)),
          this, SLOT(jobSubmissionError(const QString&)));

  request->submit();

}

void QueueUit::jobSubmittedToRemoteQueue()
{
  Uit::SubmitBatchScriptJobRequest *request
    = qobject_cast<Uit::SubmitBatchScriptJobRequest*>(sender());

  if (!request) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not SubmitBatchScriptJobRequest!"));
      return;
  }
  request->deleteLater();

  Uit::JobSubmissionInfo info = request->jobSubmissionInfo();

  if (!info.isValid()) {
    Logger::logError(tr("Invalid response from UIT server: %1")
                        .arg(info.xml()));
  }

  Job job = request->job();

  if (!info.stderr().isEmpty()) {
    Logger::logWarning(tr("Could not submit job to remote UIT queue on %1:\n"
                                "stderr: %2")
                             .arg(m_hostName)
                             .arg(info.stderr()));

    // Retry submission:
    if (addJobFailure(job.moleQueueId()))
      m_pendingSubmission.append(job.moleQueueId());

    job.setJobState(MoleQueue::Error);
    return;
  }

  IdType queueId = request->jobSubmissionInfo().jobNumber();
  job.setJobState(MoleQueue::Submitted);
  clearJobFailures(job.moleQueueId());
  job.setQueueId(queueId);
  m_jobs.insert(queueId, job.moleQueueId());
}

void QueueUit::jobSubmissionError(const QString &errorString)
{
  Uit::SubmitBatchScriptJobRequest *request
      = qobject_cast<Uit::SubmitBatchScriptJobRequest*>(sender());

  if (!request) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not SubmitBatchScriptJobRequest!"));
      return;
  }
  request->deleteLater();

  Job job = request->job();

  Logger::logWarning(tr("Could not submit job to remote UIT queue on %1:\n"
                        "%2")
                         .arg(m_hostName)
                         .arg(errorString));
  // Retry submission:
  if (addJobFailure(job.moleQueueId()))
    m_pendingSubmission.append(job.moleQueueId());

  job.setJobState(MoleQueue::Error);

  emit uitMethodError(errorString);
}

void QueueUit::requestQueueUpdate()
{
  if (m_isCheckingQueue)
    return;

  if (m_jobs.isEmpty())
    return;

  m_isCheckingQueue = true;

  Uit::GetJobsForHostForUserByNumDaysRequest *request =
    new Uit::GetJobsForHostForUserByNumDaysRequest(uitSession(), this);

  request->setHostId(m_hostID);
  request->setSearchUser(m_kerberosUserName);
  request->setUserName(m_kerberosUserName);
  // What should we set this too???
  request->setNumDays(1);

  connect(request, SIGNAL(finished()),
          this, SLOT(handleQueueUpdate()));
  connect(request, SIGNAL(error(const QString&)),
          this, SLOT(requestQueueUpdateError(const QString&)));

  request->submit();
}

void QueueUit::requestQueueUpdateError(const QString &errorString)
{
  Uit::GetJobsForHostForUserByNumDaysRequest *request
    = qobject_cast<Uit::GetJobsForHostForUserByNumDaysRequest*>(sender());

  if (!request) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                  .arg("Sender is not GetJobsForHostForUserByNumDaysRequest!"));
      return;
  }
  request->deleteLater();

  Logger::logWarning(tr("Error requesting queue data: %1)").arg(errorString));

  m_isCheckingQueue = false;

  emit uitMethodError(errorString);
}

void QueueUit::handleQueueUpdate()
{
  Uit::GetJobsForHostForUserByNumDaysRequest *request
    = qobject_cast<Uit::GetJobsForHostForUserByNumDaysRequest*>(sender());

  if (!request) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                  .arg("Sender is not GetJobsForHostForUserByNumDaysRequest!"));
      return;
  }
  request->deleteLater();

  Uit::JobEventList jobEvents = request->jobEventList(m_jobs.keys());

  if (!jobEvents.isValid()) {
    Logger::logError(tr("Invalid response from UIT server: %1")
                        .arg(jobEvents.xml()));
  }

  handleQueueUpdate(jobEvents.jobEvents());
}

bool lessThan(const Uit::JobEvent &j1, const Uit::JobEvent &j2)
{
    return j1.eventTime() < j2.eventTime();
}


void QueueUit::handleQueueUpdate(const QList<Uit::JobEvent> &jobEvents)
{
  QList<IdType> justFinished;
  QList<IdType> queueIds = m_jobs.keys();

  QMap<IdType, QList<Uit::JobEvent> > eventMap;

  // Filter JobEvents by jobId
  foreach(const Uit::JobEvent& jobEvent, jobEvents) {

    QList<Uit::JobEvent> events;

    if (!eventMap.contains(jobEvent.jobId())) {
      eventMap[jobEvent.jobId()] = QList<Uit::JobEvent>();
    }

    eventMap[jobEvent.jobId()].append(jobEvent);
  }

  foreach(IdType queueId, queueIds) {

    IdType moleQueueId = m_jobs.value(queueId, InvalidId);

    if (moleQueueId != InvalidId) {
      // Get pointer to jobmanager to lookup job
      if (!m_server) {
        Logger::logError(tr("Queue '%1' cannot locate Server instance!")
                          .arg(m_name), moleQueueId);
        m_isCheckingQueue = false;
        return;
      }

      Job job = m_server->jobManager()->lookupJobByMoleQueueId(moleQueueId);
      if (!job.isValid()) {
        Logger::logError(tr("Queue '%1' Cannot update invalid Job reference!")
                            .arg(m_name), moleQueueId);
        continue;

      }

      QList<Uit::JobEvent> events = eventMap[queueId];

      // If there are no events then we assume it has finished.
      if (events.isEmpty()) {
        beginFinalizeJob(queueId);
        continue;
      }

      Uit::JobEvent lastEvent = events.last();

      JobState currentState = jobEventToJobState(lastEvent);

      if (currentState != job.jobState())
        job.setJobState(currentState);
    }
  }

  m_isCheckingQueue = false;
}

void QueueUit::beginFinalizeJob(IdType queueId)
{
  IdType moleQueueId = m_jobs.value(queueId, InvalidId);
  if (moleQueueId == InvalidId)
    return;

  m_jobs.remove(queueId);

  // Lookup job
  if (!m_server)
    return;
  Job job = m_server->jobManager()->lookupJobByMoleQueueId(moleQueueId);
  if (!job.isValid())
    return;

  finalizeJobCopyFromServer(job);
}

void QueueUit::finalizeJobCopyFromServer(Job job)
{
  if (!job.retrieveOutput() ||
      (job.cleanLocalWorkingDirectory() && job.outputDirectory().isEmpty())
      ) {
    // Jump to next step
    finalizeJobCopyToCustomDestination(job);
    return;
  }

  QString localDir = job.localWorkingDirectory();
  QString remoteDir
    = QString("%1/%2").arg(m_workingDirectoryBase).arg(job.moleQueueId());

  Uit::DirectoryDownload *downloader = new Uit::DirectoryDownload(uitSession(),
                                                                  this);
  downloader->setJob(job);
  downloader->setHostId(m_hostID);
  downloader->setUserName(m_kerberosUserName);
  downloader->setRemotePath(remoteDir);
  downloader->setLocalPath(localDir);

  connect(downloader, SIGNAL(finished()),
          this, SLOT(finalizeJobOutputCopiedFromServer()));
  connect(downloader, SIGNAL(error(const QString &)),
          this, SLOT(finalizeJobCopyFromServerError(const QString &)));

  downloader->start();

}

void QueueUit::finalizeJobCopyFromServerError(const QString &errorString)
{
  Uit::DirectoryDownload *downloader
      = qobject_cast<Uit::DirectoryDownload*>(sender());

  if (!downloader) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                  .arg("Sender is not UitDirDownloader!"));
      return;
  }
  downloader->deleteLater();

  Job job = downloader->job();

  Logger::logError(tr("Error copy file from server: %1").arg(errorString),
                   job.moleQueueId());

  job.setJobState(MoleQueue::Error);

  emit uitMethodError(errorString);
}

void QueueUit::finalizeJobOutputCopiedFromServer()
{
  Uit::DirectoryDownload *downloader
      = qobject_cast<Uit::DirectoryDownload*>(sender());

  if (!downloader) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                  .arg("Sender is not UitDirDownloader!"));
      return;
  }
  downloader->deleteLater();

  finalizeJobCopyToCustomDestination(downloader->job());
}

// TODO This can probably move to the super class ??
void QueueUit::finalizeJobCopyToCustomDestination(Job job)
{
  // Skip to next step if needed
  if (job.outputDirectory().isEmpty() ||
      job.outputDirectory() == job.localWorkingDirectory()) {
    finalizeJobCleanup(job);
    return;
  }

  // The copy function will throw errors if needed.
  if (!FileSystemTools::recursiveCopyDirectory(job.localWorkingDirectory(),
                                               job.outputDirectory())) {
    job.setJobState(MoleQueue::Error);
    return;
  }

  finalizeJobCleanup(job);
}

void QueueUit::finalizeJobCleanup(Job job)
{
  if (job.cleanLocalWorkingDirectory())
    cleanLocalDirectory(job);

  if (job.cleanRemoteFiles())
    cleanRemoteDirectory(job);

  job.setJobState(MoleQueue::Finished);
}

void QueueUit::cleanRemoteDirectory(Job job)
{
  QString remoteDir = QDir::cleanPath(
  QString("%1/%2").arg(m_workingDirectoryBase).arg(job.moleQueueId()));

  // Check that the remoteDir is not just "/" due to another bug.
  if (remoteDir.simplified() == "/") {
    Logger::logError(tr("Refusing to clean remote directory %1 -- an internal "
                        "error has occurred.").arg(remoteDir),
                     job.moleQueueId());
    return;
  }

  Uit::DirectoryDelete *deleter = new Uit::DirectoryDelete(uitSession(), this);
  deleter->setHostId(m_hostID);
  deleter->setUserName(m_kerberosUserName);
  deleter->setJob(job);
  deleter->setDirectory(remoteDir);

  connect(deleter, SIGNAL(finished()),
          this, SLOT(remoteDirectoryCleaned()));
  connect(deleter, SIGNAL(error(const QString &)),
          this, SLOT(cleanRemoteDirectoryError(const QString &)));

}

void QueueUit::cleanRemoteDirectoryError(const QString &errorString)
{
  Uit::DirectoryDelete *deleter
      = qobject_cast<Uit::DirectoryDelete*>(sender());

  if (!deleter) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                  .arg("Sender is not UitDirDeleter!"));
      return;
  }
  deleter->deleteLater();

  Job job = deleter->job();

  Logger::logError(tr("Error clearing remote directory '%1.\n"
                      "%2").arg(deleter->directory()).arg(errorString),
                   job.moleQueueId());

  job.setJobState(MoleQueue::Error);

  emit uitMethodError(errorString);
}

void QueueUit::remoteDirectoryCleaned()
{
  Uit::DirectoryDelete *deleter
      = qobject_cast<Uit::DirectoryDelete*>(sender());

  if (!deleter) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                  .arg("Sender is not UitDirDeleter!"));
      return;
  }
  deleter->deleteLater();
}

void QueueUit::beginKillJob(Job job)
{
  Uit::CancelJobRequest *request
    = new Uit::CancelJobRequest(uitSession(), this);
  request->setHostId(m_hostID);
  request->setUserName(m_kerberosUserName);
  request->setJob(job);

  connect(request, SIGNAL(finished()),
          this, SLOT(endKillJob()));
  connect(request, SIGNAL(error(const QString&)),
          this, SLOT(killJobError(const QString&)));

  request->submit();
}

void QueueUit::killJobError(const QString &errorString)
{
  Uit::CancelJobRequest *request
    = qobject_cast<Uit::CancelJobRequest*>(sender());

  if (!request) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                  .arg("Sender is not CancelJobRequest!"));
      return;
  }
  request->deleteLater();

  Job job = request->job();

  Logger::logWarning(tr("Error canceling job (mqid=%1, queueid=%2) %3 ")
                     .arg(job.moleQueueId()).arg(job.queueId())
                     .arg(errorString));

  emit uitMethodError(errorString);
}


void QueueUit::endKillJob()
{
  Uit::CancelJobRequest *request
    = qobject_cast<Uit::CancelJobRequest*>(sender());

  if (!request) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                  .arg("Sender is not CancelJobRequest!"));
      return;
  }
  request->deleteLater();

  Job job = request->job();
  job.setJobState(MoleQueue::Canceled);
}

void QueueUit::getUserHostAssoc()
{
  Uit::GetUserHostAssocRequest *request
    = new Uit::GetUserHostAssocRequest(uitSession(), this);

  connect(request, SIGNAL(finished()),
          this, SLOT(getUserHostAssocComplete()));
  connect(request, SIGNAL(error(const QString&)),
           this, SLOT(requestError(const QString&)));

  request->submit();
}

void QueueUit::getUserHostAssocComplete()
{
  Uit::GetUserHostAssocRequest *request
    = qobject_cast<Uit::GetUserHostAssocRequest*>(sender());

  if (!request) {
    Logger::logError(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                     .arg("Sender is not GetUserHostAssocRequest!"));
      return;
  }
  request->deleteLater();

  Uit::UserHostAssocList userHostAssoc = request->userHostAssocList();

  if (!userHostAssoc.isValid()) {
    Logger::logError(tr("Invalid response from UIT server: %1")
                     .arg(userHostAssoc.xml()));
    return;
  }

  emit userHostAssocList(userHostAssoc);
}

Uit::Session * QueueUit::uitSession()
{
  if (!m_uitSession)
    m_uitSession = Uit::SessionManager::instance()->session(m_kerberosUserName,
                                                            m_kerberosRealm);

  return m_uitSession;
}

void QueueUit::requestError(const QString &errorMessage)
{
  Uit::Request *request
     = qobject_cast<Uit::Request*>(sender());

  if (!request) {
    Logger::logWarning(tr("Internal error: %1\n%2").arg(Q_FUNC_INFO)
                      .arg("Sender is not UitRequest!"));
     return;
  }
  request->deleteLater();

  emit uitMethodError(errorMessage);
}

JobState QueueUit::jobEventToJobState(Uit::JobEvent jobEvent)
{
  QString jobStatus = jobEvent.jobStatusText().trimmed();

  if (jobStatus.length() != 1) {
    Logger::logError(tr("Unrecognized jobStatus: %1").arg(jobStatus));
    return MoleQueue::Error;
  }

  JobState jobState = Unknown;

  char state = jobStatus.toLower()[0].toLatin1();

  switch (state) {
    case 'r':
    case 'e':
    case 'c':
      jobState = RunningRemote;
      break;
    case 'q':
    case 'h':
    case 't':
    case 'w':
    case 's':
      jobState = QueuedRemote;
      break;
    default:
      Logger::logWarning(tr("Unrecognized queue state '%1'.").arg(state));
  }

  return jobState;
}

} // End namespace
