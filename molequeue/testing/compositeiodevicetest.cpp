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

#include "queues/uit/compositeiodevice.h"
#include "referencestring.h"
#include "molequeuetestconfig.h"

class CompositeIODeviceTest : public QObject
{
  Q_OBJECT
private slots:
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void testReadAll();
  void testReadSome();
  void testReadOver();
  void testReadBytes();
  void testSize();
  void testUploadPattern();

private:
  MoleQueue::Uit::CompositeIODevice *m_comp;
  QByteArray *byteArray1;
  QByteArray *byteArray2;
};

void CompositeIODeviceTest::init()
{
  m_comp = new MoleQueue::Uit::CompositeIODevice(this);
  m_comp->open(QIODevice::ReadOnly);

  byteArray1 = new QByteArray("abc");
  QBuffer *buffer1 = new QBuffer(byteArray1, this);
  buffer1->open(QIODevice::ReadOnly);
  m_comp->addDevice(buffer1);

  byteArray2 = new QByteArray("def");
  QBuffer *buffer2 = new QBuffer(byteArray2, this);
  buffer2->open(QIODevice::ReadOnly);
  m_comp->addDevice(buffer2);
}

void CompositeIODeviceTest::cleanup()
{
  delete m_comp;
  delete byteArray1;
  delete byteArray2;
}

void CompositeIODeviceTest::testReadAll()
{
  char data[6];

  QVERIFY(m_comp->read(data, 6) == 6);
  QCOMPARE(QString::fromLocal8Bit(data, 6), QString("abcdef"));
}

void CompositeIODeviceTest::testReadSome()
{
  char data[2];

  QVERIFY(m_comp->read(data, 2) == 2);
  QCOMPARE(QString::fromLocal8Bit(data, 2), QString("ab"));
}

void CompositeIODeviceTest::testReadOver()
{
  char data[6];

  QVERIFY(m_comp->read(data, 100) == 6);
  QCOMPARE(QString::fromLocal8Bit(data, 6), QString("abcdef"));
}


void CompositeIODeviceTest::testReadBytes()
{
  char c;
  QByteArray bytes;
  while(m_comp->read(&c, 1) != -1)
    bytes.append(c);
  QCOMPARE(QString(bytes), QString("abcdef"));
}

void CompositeIODeviceTest::testSize()
{
  QVERIFY(m_comp->size() == 6);
  QString testFilePath = "compositeiodevice-ref/testfile.txt";
  ReferenceString fileString(testFilePath);
  QFile file(MoleQueue_TESTDATA_DIR + testFilePath);
  file.open(QIODevice::ReadOnly);

  m_comp->addDevice(&file);

  QVERIFY(m_comp->size() == file.size()+6);
  QCOMPARE(QString(m_comp->readAll()), QString("abcdef")+fileString);

}

void CompositeIODeviceTest::testUploadPattern()
{
  QString xml = "<header> </header>";
  QString testFilePath = "compositeiodevice-ref/testfile.txt";
  ReferenceString fileString(testFilePath);
  QFile file(MoleQueue_TESTDATA_DIR + testFilePath);
  file.open(QIODevice::ReadWrite);
  MoleQueue::Uit::CompositeIODevice *dataStream
    = new MoleQueue::Uit::CompositeIODevice(this);
  dataStream->open(QIODevice::ReadWrite);
  QBuffer *headerBuffer = new QBuffer(dataStream);
  headerBuffer->open(QIODevice::ReadWrite);
  QTextStream headerStream(headerBuffer);

  headerStream << xml.size();
  headerStream << "|";
  headerStream << xml;
  headerStream << file.size();
  headerStream << "|";
  headerStream.flush();

  headerBuffer->seek(0);

  dataStream->addDevice(headerBuffer);
  dataStream->addDevice(&file);

  QByteArray bytes;

  char c;

  while(dataStream->read(&c, 1) > 0) {
    bytes.append(c);
  }

  QCOMPARE(QString(bytes), QString("%1|<header> </header>%2|%3")
                           .arg(xml.length())
                           .arg(QString(fileString).length())
                           .arg(fileString));

}

QTEST_MAIN(CompositeIODeviceTest)

#include "compositeiodevicetest.moc"
