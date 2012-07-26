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

#include "dummysshcommand.h"

class SshCommandTest : public QObject
{
  Q_OBJECT

private:
  DummySshCommand m_ssh;

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void sanityCheck();
  void testExecute();
  void testCopyTo();
  void testCopyFrom();
  void testCopyDirTo();
  void testCopyDirFrom();
};

void SshCommandTest::initTestCase()
{
}

void SshCommandTest::cleanupTestCase()
{
}

void SshCommandTest::init()
{
  m_ssh.setSshCommand("ssh");
  m_ssh.setScpCommand("scp");
  m_ssh.setHostName("host");
  m_ssh.setUserName("user");
  m_ssh.setPortNumber(22);
}

void SshCommandTest::cleanup()
{
}

void SshCommandTest::sanityCheck()
{
  m_ssh.setSshCommand("mySsh");
  QCOMPARE(m_ssh.sshCommand(), QString("mySsh"));
  m_ssh.setScpCommand("myScp");
  QCOMPARE(m_ssh.scpCommand(), QString("myScp"));
  m_ssh.setData(QVariant("Test"));
  QCOMPARE(m_ssh.data().toString(), QString("Test"));
}

void SshCommandTest::testExecute()
{
  m_ssh.execute("ls ~");
  QCOMPARE(m_ssh.getDummyCommand(), QString("ssh"));
  QCOMPARE(m_ssh.getDummyArgs(), QStringList ()
           << QString("-q")
           << QString("-p") << QString("22")
           << QString("user@host")
           << QString("ls ~"));
}

void SshCommandTest::testCopyTo()
{
  m_ssh.copyTo("C:/local/path", "/remote/path");
  QCOMPARE(m_ssh.getDummyCommand(), QString("scp"));
  QCOMPARE(m_ssh.getDummyArgs(), QStringList ()
           << QString("-q")
           << QString("-P") << QString("22")
           << QString("C:/local/path")
           << QString("user@host:/remote/path"));
}

void SshCommandTest::testCopyFrom()
{
  m_ssh.copyFrom("/remote/path", "C:/local/path");
  QCOMPARE(m_ssh.getDummyCommand(), QString("scp"));
  QCOMPARE(m_ssh.getDummyArgs(), QStringList ()
           << QString("-q")
           << QString("-P") << QString("22")
           << QString("user@host:/remote/path")
           << QString("C:/local/path"));
}

void SshCommandTest::testCopyDirTo()
{
  m_ssh.copyDirTo("C:/local/path", "/remote/path");
  QCOMPARE(m_ssh.getDummyCommand(), QString("scp"));
  QCOMPARE(m_ssh.getDummyArgs(), QStringList ()
           << QString("-q")
           << QString("-P") << QString("22")
           << QString("-r")
           << QString("C:/local/path")
           << QString("user@host:/remote/path"));
}

void SshCommandTest::testCopyDirFrom()
{
  m_ssh.copyDirFrom("/remote/path", "C:/local/path");
  QCOMPARE(m_ssh.getDummyCommand(), QString("scp"));
  QCOMPARE(m_ssh.getDummyArgs(), QStringList ()
           << QString("-q")
           << QString("-P") << QString("22")
           << QString("-r")
           << QString("user@host:/remote/path")
           << QString("C:/local/path"));
}

QTEST_MAIN(SshCommandTest)

#include "moc_sshcommandtest.cxx"
