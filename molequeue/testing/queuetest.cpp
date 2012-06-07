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

#include <QtTest>

#include "program.h"
#include "queue.h"

class DummyQueue : public MoleQueue::Queue
{
  Q_OBJECT
public:
  DummyQueue(const QString &queueName = "Dummy")
    : MoleQueue::Queue(queueName, NULL) {};
public slots:
  virtual bool submitJob(const MoleQueue::Job *) {return false;}
};

class QueueTest : public QObject
{
  Q_OBJECT

private:
  DummyQueue m_queue;

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void testNames();
  void testAddProgram();
  void testLookupProgram();
  void testNumPrograms();
  void testProgramNames();
  void testRemoveProgram();
  void testCleanup();
};

void QueueTest::initTestCase()
{
}

void QueueTest::cleanupTestCase()
{
}

void QueueTest::init()
{
}

void QueueTest::cleanup()
{
}

void QueueTest::testNames()
{
  QCOMPARE(m_queue.name(), QString ("Dummy"));
  m_queue.setName("SomeQueue");
  QCOMPARE(m_queue.name(), QString ("SomeQueue"));
}

void QueueTest::testAddProgram()
{
  QSignalSpy spy (&m_queue, SIGNAL(programAdded(QString,MoleQueue::Program*)));

  MoleQueue::Program *p1 = new MoleQueue::Program(&m_queue);
  p1->setName("First Program");
  MoleQueue::Program *p2 = new MoleQueue::Program(NULL);
  p2->setName("Second Program");
  MoleQueue::Program *p2a = new MoleQueue::Program(&m_queue);
  p2a->setName("Second Program");

  QVERIFY(m_queue.addProgram(p1));
  QVERIFY(m_queue.addProgram(p2));
  QVERIFY(!m_queue.addProgram(p2a)); // Duplicate name

  QCOMPARE(spy.count(), 2);
}

void QueueTest::testLookupProgram()
{
  QString programName ("First Program");
  QCOMPARE(m_queue.lookupProgram(programName)->name(), programName);
}

void QueueTest::testNumPrograms()
{
  QCOMPARE(m_queue.numPrograms(), 2);
}

void QueueTest::testProgramNames()
{
  QStringList programNames = m_queue.programNames();
  qSort(programNames);
  QCOMPARE(programNames.size(), 2);
  QCOMPARE(programNames[0], QString("First Program"));
  QCOMPARE(programNames[1], QString("Second Program"));
}

void QueueTest::testRemoveProgram()
{
  QSignalSpy spy (&m_queue, SIGNAL(programRemoved(QString,MoleQueue::Program*)));

  MoleQueue::Program notInQueue;
  QCOMPARE(m_queue.removeProgram(&notInQueue ), false);
  QCOMPARE(m_queue.removeProgram("notInQueue"), false);

  QCOMPARE(m_queue.removeProgram("First Program"), true);
  QCOMPARE(m_queue.numPrograms(), 1);
  QCOMPARE(m_queue.removeProgram(m_queue.programs().first()), true);
  QCOMPARE(m_queue.numPrograms(), 0);

  QCOMPARE(spy.count(), 2);
}

void QueueTest::testCleanup()
{
  DummyQueue *queue = new DummyQueue();

  QPointer<MoleQueue::Program> program = new MoleQueue::Program ();
  queue->addProgram(program.data());
  delete queue;
  queue = NULL;

  QCOMPARE(program.data(), static_cast<MoleQueue::Program*>(NULL));
}

QTEST_MAIN(QueueTest)

#include "moc_queuetest.cxx"
