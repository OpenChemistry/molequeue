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

class QueueTest : public QObject
{
  Q_OBJECT

private:

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void runTest();
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

void QueueTest::runTest()
{
  bool error = false;
  qDebug() << "Testing the queue class...";

  MoleQueue::Program *gamess = new MoleQueue::Program;
  gamess->setName("GAMESS");
//  gamess.setReplacement("input", "myInput.inp");
//  gamess.setReplacement("ncpus", "8");
  gamess->setRunTemplate("rungms $$input$$ 2010 $$ncpus$$");

  MoleQueue::Program *gaussian = new MoleQueue::Program;
  gaussian->setName("Gaussian");
//  gaussian.setReplacement("input", "input.com");
  gaussian->setRunTemplate("gaussian $$input$$");

  MoleQueue::Queue queue;
  if (!queue.addProgram(gamess)) {
    error = true;
    qDebug() << "Error adding the gamess program to the queue.";
  }
  if (!queue.addProgram(gaussian)) {
    error = true;
    qDebug() << "Error adding the gaussian program to the queue.";
  }
  QStringList programs = queue.programs();
  qDebug() << "Programs in queue: " << programs.join(" ");

  if (!queue.removeProgram("GAMESS")) {
    error = true;
    qDebug() << "Error removing the GAMESS program from the queue.";
  }

  foreach (const QString &name, programs)
    qDebug() << name;

  programs = queue.programs();
  qDebug() << "Programs in queue: " << programs.join(" ");

  QVERIFY(error == false);
}

QTEST_MAIN(QueueTest)

#include "moc_queuetest.cxx"
