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

#include "jobmanager.h"

#include "job.h"
#include "molequeueglobal.h"

#include <QtTest>

using MoleQueue::Job;

class JobManagerTest : public QObject
{
  Q_OBJECT

private:
  MoleQueue::JobManager m_jobManager;

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  // Set both MoleQueue and Client ids to the current count()+1
  void setNewJobIds(Job*);

  void testJobAboutToBeAdded();
  void testlookupClientId();
  void testlookupMoleQueueId();

};

void JobManagerTest::initTestCase()
{
  connect(&m_jobManager, SIGNAL(jobAboutToBeAdded(Job*)),
          this, SLOT(setNewJobIds(Job*)),
          Qt::DirectConnection);
}

void JobManagerTest::cleanupTestCase()
{
}

void JobManagerTest::init()
{
}

void JobManagerTest::cleanup()
{
}

void JobManagerTest::setNewJobIds(MoleQueue::Job *job)
{
  MoleQueue::IdType id = static_cast<MoleQueue::IdType>(m_jobManager.count())+1;
  job->setMolequeueId(id);
  job->setClientId(id);
}

void JobManagerTest::testJobAboutToBeAdded()
{
  QSignalSpy spy (&m_jobManager, SIGNAL(jobAboutToBeAdded(Job*)));

  m_jobManager.newJob();
  QCOMPARE(spy.count(), 1);

  m_jobManager.newJob(m_jobManager.jobs().last()->hash());
  QCOMPARE(spy.count(), 2);
}

void JobManagerTest::testlookupClientId()
{
  if (m_jobManager.jobs().size() != 2) {
    qDebug() << "Not enough jobs in the queuemanager. Skipping test.";
    return;
  }
  const MoleQueue::Job *job1 = m_jobManager.jobs()[0];
  const MoleQueue::Job *job2 = m_jobManager.jobs()[1];

  QCOMPARE(job1, m_jobManager.lookupClientId(1));
  QCOMPARE(job2, m_jobManager.lookupClientId(2));
}

void JobManagerTest::testlookupMoleQueueId()
{
  if (m_jobManager.jobs().size() != 2) {
    qDebug() << "Not enough jobs in the queuemanager. Skipping test.";
    return;
  }
  const MoleQueue::Job *job1 = m_jobManager.jobs()[0];
  const MoleQueue::Job *job2 = m_jobManager.jobs()[1];

  QCOMPARE(job1, m_jobManager.lookupMoleQueueId(1));
  QCOMPARE(job2, m_jobManager.lookupMoleQueueId(2));
}

QTEST_MAIN(JobManagerTest)

#include "moc_jobmanagertest.cxx"
