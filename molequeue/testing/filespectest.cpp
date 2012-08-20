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

#include "filespec.h"

#include <json/json.h>

#include <QtCore/QTemporaryFile>

using namespace MoleQueue;

class FileSpecTest : public QObject
{
  Q_OBJECT

private:

  QString readReferenceString(const QString &filename_);

private slots:
  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void ctorFromVariantHash();
  void ctorFromPath();
  void ctorFromFileNameAndContents();
  void ctorFromFile();
  void ctorCopy();
  void assignment();

  void format();
  void isValid();
  void asVariantHash();
  void fileExists();
  void writeFile();
  void filename();
  void contents();
  void filepath();
  void fileHasExtension();
  void fileBaseName();
  void fileExtension();

};

QString FileSpecTest::readReferenceString(const QString &filename_)
{
  QString realFilename = TESTDATADIR + filename_;
  QFile refFile (realFilename);
  if (!refFile.open(QFile::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot access reference file" << realFilename;
    return "";
  }
  QString result = QString(refFile.readAll());
  refFile.close();
  return result;
}

void FileSpecTest::initTestCase()
{
}

void FileSpecTest::cleanupTestCase()
{
}

void FileSpecTest::init()
{
}

void FileSpecTest::cleanup()
{
}

void FileSpecTest::ctorFromVariantHash()
{
  QVariantHash path;
  path["filepath"] = QString("/some/path/to/a/file.ext");

  FileSpec pathSpec(path);
  QString ref = readReferenceString("filespec-ref/path.json");
  QCOMPARE(pathSpec.asJsonString(), ref);
}

void FileSpecTest::ctorFromPath()
{
  FileSpec pathSpec(QString("/some/path/to/a/file.ext"));
  QString ref = readReferenceString("filespec-ref/path.json");
  QCOMPARE(pathSpec.asJsonString(), ref);
}

void FileSpecTest::ctorFromFileNameAndContents()
{
  FileSpec contSpec(QString("file.ext"), QString("I'm input file text!\n"));
  QString ref = readReferenceString("filespec-ref/contents.json");
  QCOMPARE(contSpec.asJsonString(), ref);
}

void FileSpecTest::ctorFromFile()
{
  QTemporaryFile file;
  QVERIFY(file.open());

  QByteArray content("I'm input file text!!\n");
  file.write(content);

  file.close();

  FileSpec spec(&file, FileSpec::PathFileSpec);
  QCOMPARE(spec.format(), FileSpec::PathFileSpec);
  QCOMPARE(spec.filepath(), QFileInfo(file).absoluteFilePath());

  spec = FileSpec(&file, FileSpec::ContentsFileSpec);
  QCOMPARE(spec.format(), FileSpec::ContentsFileSpec);
  QCOMPARE(spec.filename(), QFileInfo(file).fileName());
  QCOMPARE(spec.contents(), QString(content));
}

void FileSpecTest::ctorCopy()
{
  FileSpec spec1(QString("/path/to/some/file.ext"));
  FileSpec spec2(spec1);

  QCOMPARE(spec1.asJsonString(), spec2.asJsonString());
}

void FileSpecTest::assignment()
{
  FileSpec spec1(QString("/path/to/some/file.ext"));
  FileSpec spec2;
  spec2 = spec1;

  QCOMPARE(spec1.asJsonString(), spec2.asJsonString());
}

void FileSpecTest::format()
{
  FileSpec pathSpec(QString("/some/path/to/a/file.ext"));
  QCOMPARE(pathSpec.format(), FileSpec::PathFileSpec);

  FileSpec contSpec(QString("file.ext"), QString("I'm input file text!\n"));
  QCOMPARE(contSpec.format(), FileSpec::ContentsFileSpec);

  QVariantHash hash;

  FileSpec inv1(hash);
  QCOMPARE(inv1.format(), FileSpec::InvalidFileSpec);

  hash.insert("notARealKey", "Bad value!");
  FileSpec inv2(hash);
  QCOMPARE(inv2.format(), FileSpec::InvalidFileSpec);

  // filename, but no contents
  hash.insert("filename", "Bad value!");
  FileSpec inv3(hash);
  QCOMPARE(inv3.format(), FileSpec::InvalidFileSpec);

  FileSpec inv4;
  QCOMPARE(inv4.format(), FileSpec::InvalidFileSpec);
}

void FileSpecTest::isValid()
{
  FileSpec pathSpec(QString("/some/path/to/a/file.ext"));
  QVERIFY(pathSpec.isValid());

  FileSpec contSpec(QString("file.ext"), QString("I'm input file text!\n"));
  QVERIFY(contSpec.isValid());

  QVariantHash hash;
  FileSpec inv(hash);
  QVERIFY(!inv.isValid());
}

void FileSpecTest::asVariantHash()
{
  FileSpec pathSpec(QString("/some/path/to/a/file.ext"));
  QVariantHash pathHash = pathSpec.asVariantHash();
  QCOMPARE(pathHash["filepath"].toString(),
           QString("/some/path/to/a/file.ext"));

  FileSpec contSpec(QString("file.ext"), QString("I'm input file text!\n"));
  QVariantHash contHash = contSpec.asVariantHash();
  QCOMPARE(contHash["filename"].toString(),
           QString("file.ext"));
  QCOMPARE(contHash["contents"].toString(),
           QString("I'm input file text!\n"));
}

void FileSpecTest::fileExists()
{
  QTemporaryFile file;
  // filename isn't generated until open is called.
  file.open();

  FileSpec spec(&file, FileSpec::PathFileSpec);
  QVERIFY(spec.fileExists());

  /// Always returns false for contents, since no path is known.
  spec = FileSpec(&file, FileSpec::ContentsFileSpec);
  QVERIFY(!spec.fileExists());

  file.close();
}

void FileSpecTest::writeFile()
{
  QTemporaryFile file;
  // filename isn't available until open is called.
  file.open();

  QString content("I'm sample input file contents!\n");
  FileSpec spec(file.fileName(), content);

  spec.writeFile(QFileInfo(file).dir());
  QCOMPARE(QString(file.readAll()), content);
  file.close();
}

void FileSpecTest::filename()
{
  FileSpec contSpec("file.ext", "contents\n");
  QCOMPARE(contSpec.filename(), QString("file.ext"));

  FileSpec pathSpec(QString("/path/to/some/file.ext"));
  QCOMPARE(pathSpec.filename(), QString("file.ext"));
}

void FileSpecTest::contents()
{
  QTemporaryFile file;
  // filename isn't available until open is called.
  file.open();
  QString content("I'm sample input file contents!\n");
  FileSpec spec(file.fileName(), content);
  QCOMPARE(spec.contents(), content);

  spec.writeFile(QFileInfo(file).dir());
  QCOMPARE(spec.contents(), content);
  file.close();
}

void FileSpecTest::filepath()
{
  FileSpec pathSpec(QString("/path/to/some/file.ext"));
  QCOMPARE(pathSpec.filepath(), QString("/path/to/some/file.ext"));

  FileSpec contSpec("file.ext", "contents\n");
  QVERIFY(contSpec.filepath().isNull());
}

void FileSpecTest::fileHasExtension()
{
  FileSpec pathSpec(QString("/path/to/some/file.ext"));
  QVERIFY(pathSpec.fileHasExtension());
  pathSpec = FileSpec(QString("/path/to/some/file"));
  QVERIFY(!pathSpec.fileHasExtension());

  FileSpec contSpec("file.ext", "contents\n");
  QVERIFY(contSpec.fileHasExtension());
  contSpec = FileSpec("file", "contents\n");
  QVERIFY(!contSpec.fileHasExtension());
}

void FileSpecTest::fileBaseName()
{
  FileSpec pathSpec(QString("/path/to/some/file.ext"));
  QCOMPARE(pathSpec.fileBaseName(), QString("file"));
  pathSpec = FileSpec(QString("/path/to/some/file"));
  QCOMPARE(pathSpec.fileBaseName(), QString("file"));

  FileSpec contSpec("file.ext", "contents\n");
  QCOMPARE(contSpec.fileBaseName(), QString("file"));
  contSpec = FileSpec("file", "contents\n");
  QCOMPARE(contSpec.fileBaseName(), QString("file"));
}

void FileSpecTest::fileExtension()
{
  FileSpec pathSpec(QString("/path/to/some/file.ext"));
  QCOMPARE(pathSpec.fileExtension(), QString("ext"));
  pathSpec = FileSpec(QString("/path/to/some/file"));
  QVERIFY(pathSpec.fileExtension().isNull());

  FileSpec contSpec("file.ext", "contents\n");
  QCOMPARE(contSpec.fileExtension(), QString("ext"));
  contSpec = FileSpec("file", "contents\n");
  QVERIFY(contSpec.fileExtension().isNull());
}

QTEST_MAIN(FileSpecTest)

#include "filespectest.moc"
