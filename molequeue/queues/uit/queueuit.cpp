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

#include "queueuit.h"

#include "kerberoscredentials.h"
#include "sslsetup.h"
#include "uitauthenticator.h"

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

#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtCore/QXmlStreamWriter>
#include <QtCore/QSettings>
#include <QtXmlPatterns/QAbstractMessageHandler>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

namespace MoleQueue
{

const QString QueueUit::clientId = "0adc5b59-5827-4331-a544-5ba7922ec2b8";

QueueUit::QueueUit(QueueManager *parentObject)
  : QueueRemote("ezHPC UIT", parentObject), m_dialogParent(NULL)
{
  // ensure SSL certificates are loaded
  SslSetup::init();
}

QueueUit::~QueueUit()
{
}

void QueueUit::readSettings(QSettings &settings)
{
  QueueRemote::readSettings(settings);

  m_kerberosPrinciple = settings.value("kerberosPrinciple").toString();
  m_hostName = settings.value("kerberosHostName").toString();
}

void QueueUit::writeSettings(QSettings &settings) const
{
  QueueRemote::writeSettings(settings);

  settings.setValue("kerberosPrinciple", m_kerberosPrinciple);
  settings.setValue("kerberosHostName", m_hostName);
}

void QueueUit::exportConfiguration(QSettings &exporter,
                                   bool includePrograms) const
{
  QueueRemote::exportConfiguration(exporter, includePrograms);

  // TODO
}

void QueueUit::importConfiguration(QSettings &importer,
                                   bool includePrograms)
{
  QueueRemote::importConfiguration(importer, includePrograms);

  // TODO
}

bool QueueUit::testConnection(QWidget *parentObject)
{
  UitAuthenticator *authenticator = new UitAuthenticator(
                                          &m_uit, m_kerberosPrinciple,
                                          this, parentObject);

  m_dialogParent = parentObject;

  connect(authenticator, SIGNAL(authenticationComplete(const QString&)),
          this, SLOT(testConnectionComplete(const QString&)));
  connect(authenticator, SIGNAL(authenticationError(const QString&)),
            this, SLOT(testConnectionError(const QString&)));

  authenticator->authenticate();

  return true;
}

void QueueUit::testConnectionComplete(const QString &token)
{
  UitAuthenticator *auth = qobject_cast<UitAuthenticator *>(sender());

  if(auth)
    auth->deleteLater();

  QMessageBox::information(m_dialogParent, tr("Success"),
                           tr("Connection to UIT succeeded!"));

  Q_UNUSED(token);
}

void QueueUit::testConnectionError(const QString &errorMessage)
{
  UitAuthenticator *auth = qobject_cast<UitAuthenticator*>(sender());

  if(auth)
    auth->deleteLater();

  QMessageBox::critical(m_dialogParent, tr("UIT Error"), errorMessage);
}

AbstractQueueSettingsWidget* QueueUit::settingsWidget()
{
  UitQueueWidget *widget = new UitQueueWidget (this);
  return widget;
}

void QueueUit::createRemoteDirectory(Job job)
{
  Q_UNUSED(job);
  // TODO
}

void QueueUit::remoteDirectoryCreated()
{
  // TODO
}

void QueueUit::copyInputFilesToHost(Job job)
{
  Q_UNUSED(job);
  //TODO
}

void QueueUit::inputFilesCopied()
{
  // TODO
}

void QueueUit::submitJobToRemoteQueue(Job job)
{
  Q_UNUSED(job);
  // TODO
}

void QueueUit::jobSubmittedToRemoteQueue()
{
  // TODO
}

void QueueUit::requestQueueUpdate()
{
  // TODO
}

void QueueUit::handleQueueUpdate()
{
  // TODO
}

void QueueUit::beginFinalizeJob(IdType queueId)
{
  Q_UNUSED(queueId);
  // TODO
}

void QueueUit::finalizeJobCopyFromServer(Job job)
{
  Q_UNUSED(job);
  // TODO
}

void QueueUit::finalizeJobOutputCopiedFromServer()
{
  // TODO
}

void QueueUit::finalizeJobCopyToCustomDestination(Job job)
{
  Q_UNUSED(job);
  // TODO
}

void QueueUit::finalizeJobCleanup(Job job)
{
  Q_UNUSED(job);
  // TODO
}

void QueueUit::cleanRemoteDirectory(Job job)
{
  Q_UNUSED(job);
  // TODO
}

void QueueUit::remoteDirectoryCleaned()
{
  // TODO
}

void QueueUit::beginKillJob(Job job)
{
  Q_UNUSED(job);
  // TODO
}

void QueueUit::endKillJob()
{
  // TODO
}

} // End namespace
