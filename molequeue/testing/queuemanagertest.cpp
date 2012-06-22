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

#include <QtTest>

#include <QtCore/QPointer>
#include <QtCore/QList>

class QueueDummy : public MoleQueue::Queue
{
  Q_OBJECT
public:
  QueueDummy(MoleQueue::QueueManager *parentManager)
    : MoleQueue::Queue ("Dummy", parentManager)
  {
  }
public slots:
  bool submitJob(const MoleQueue::Job *) {return false;}
};

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
  const QStringList & queues = m_queueManager.availableQueues();
  QVERIFY(queues.size());
  QVERIFY(m_queueManager.addQueue("First Queue", queues.first()) != NULL);
  QVERIFY(m_queueManager.addQueue("Second Queue", queues.first()) != NULL);
  QVERIFY(m_queueManager.addQueue("Second Queue", queues.first()) == NULL); // duplicate name

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

  QueueDummy notInManager (NULL);
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
  const QStringList & queues = manager->availableQueues();
  QVERIFY(queues.size());

  QPointer<MoleQueue::Queue> q = manager->addQueue("", queues.first());
  delete manager;
  manager = NULL;

  QCOMPARE(q.data(), static_cast<MoleQueue::Queue*>(NULL));
}

QTEST_MAIN(QueueManagerTest)

#include "moc_queuemanagertest.cxx"
