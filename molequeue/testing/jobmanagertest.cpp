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

  // Set MoleQueue id to the current count()+1
  void setNewJobIds(MoleQueue::Job);

  void testJobAboutToBeAdded();
  void testLookupMoleQueueId();

};

void JobManagerTest::initTestCase()
{
  connect(&m_jobManager, SIGNAL(jobAboutToBeAdded(MoleQueue::Job)),
          this, SLOT(setNewJobIds(MoleQueue::Job)),
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

void JobManagerTest::setNewJobIds(MoleQueue::Job job)
{
  MoleQueue::IdType id = static_cast<MoleQueue::IdType>(m_jobManager.count());
  job.setMoleQueueId(id);
}

void JobManagerTest::testJobAboutToBeAdded()
{
  QSignalSpy spy (&m_jobManager, SIGNAL(jobAboutToBeAdded(MoleQueue::Job)));

  m_jobManager.newJob();
  QCOMPARE(spy.count(), 1);

  m_jobManager.newJob(m_jobManager.jobAt(m_jobManager.count()-1).toJsonObject());
  QCOMPARE(spy.count(), 2);
}

void JobManagerTest::testLookupMoleQueueId()
{
  if (m_jobManager.count() != 2) {
    qDebug() << "Not enough jobs in the queuemanager. Skipping test.";
    return;
  }
  const MoleQueue::Job job1 = m_jobManager.jobAt(0);
  const MoleQueue::Job job2 = m_jobManager.jobAt(1);

  const MoleQueue::Job lookupJob1 = m_jobManager.lookupJobByMoleQueueId(1);
  const MoleQueue::Job lookupJob2 = m_jobManager.lookupJobByMoleQueueId(2);

  QCOMPARE(job1, lookupJob1);
  QCOMPARE(job2, lookupJob2);
}

QTEST_MAIN(JobManagerTest)

#include "jobmanagertest.moc"
