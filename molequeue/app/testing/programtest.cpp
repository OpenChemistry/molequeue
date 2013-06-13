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

class ProgramTest : public QObject
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

void ProgramTest::initTestCase()
{
}

void ProgramTest::cleanupTestCase()
{
}

void ProgramTest::init()
{
}

void ProgramTest::cleanup()
{
}

void ProgramTest::runTest()
{
  qDebug() << "No tests implemented yet!";
}

QTEST_MAIN(ProgramTest)

#include "programtest.moc"
