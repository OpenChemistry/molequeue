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

#include "queuemanager.h"
#include "queue.h"
#include "queues/local.h"

#include <QtTest>

#include <QtCore/QPointer>
#include <QtCore/QList>

class QueueManagerTest : public QObject
{
  Q_OBJECT

private:
  MoleQueue::QueueManager m_queueManager;

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void testAddQueue();
  void testLookupQueue();
  void testNumQueues();
  void testToQueueList();
  void testRemoveQueue();
  void testCleanup();
};

void QueueManagerTest::initTestCase()
{
}

void QueueManagerTest::cleanupTestCase()
{
}

void QueueManagerTest::init()
{
}

void QueueManagerTest::cleanup()
{
}

void QueueManagerTest::testAddQueue()
{
  QSignalSpy spy (&m_queueManager, SIGNAL(queueAdded(QString,MoleQueue::Queue*)));
  MoleQueue::Queue *q1 = new MoleQueue::QueueLocal (NULL);
  q1->setName("First Queue");
  MoleQueue::Queue *q2 = new MoleQueue::QueueLocal (&m_queueManager);
  q2->setName("Second Queue");
  MoleQueue::Queue *q2a = new MoleQueue::QueueLocal (NULL);
  q2a->setName("Second Queue");
  QCOMPARE(m_queueManager.addQueue(q1), true);
  QCOMPARE(m_queueManager.addQueue(q2), true);
  QCOMPARE(m_queueManager.addQueue(q2a), false); // duplicate name

  QCOMPARE(spy.count(), 2);
}

void QueueManagerTest::testLookupQueue()
{
  QString queueName ("First Queue");
  QCOMPARE(m_queueManager.lookupQueue(queueName)->name(), queueName);
}

void QueueManagerTest::testNumQueues()
{
  QCOMPARE(m_queueManager.numQueues(), 2);
}

void QueueManagerTest::testToQueueList()
{
  MoleQueue::QueueListType list = m_queueManager.toQueueList();
  QStringList queueNames = list.keys();
  qSort(queueNames);
  QCOMPARE(queueNames.size(), 2);
  QCOMPARE(queueNames[0], QString("First Queue"));
  QCOMPARE(queueNames[1], QString("Second Queue"));
}

void QueueManagerTest::testRemoveQueue()
{
  QSignalSpy spy (&m_queueManager, SIGNAL(queueRemoved(QString,MoleQueue::Queue*)));

  MoleQueue::QueueLocal notInManager;
  QCOMPARE(m_queueManager.removeQueue(&notInManager), false);
  QCOMPARE(m_queueManager.removeQueue("notInManager"), false);

  QCOMPARE(m_queueManager.removeQueue("First Queue"), true);
  QCOMPARE(m_queueManager.numQueues(), 1);
  QCOMPARE(m_queueManager.removeQueue(m_queueManager.queues().first()), true);
  QCOMPARE(m_queueManager.numQueues(), 0);

  QCOMPARE(spy.count(), 2);
}

void QueueManagerTest::testCleanup()
{
  MoleQueue::QueueManager *manager = new MoleQueue::QueueManager ();

  QPointer<MoleQueue::Queue> q = new MoleQueue::QueueLocal ();
  manager->addQueue(q.data());
  delete manager;
  manager = NULL;

  QCOMPARE(q.data(), static_cast<MoleQueue::Queue*>(NULL));
}

QTEST_MAIN(QueueManagerTest)

#include "moc_queuemanagertest.cxx"
