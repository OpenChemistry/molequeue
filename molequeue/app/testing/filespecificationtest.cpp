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

#include "filespecification.h"
#include "molequeuetestconfig.h"

#include <qjsonobject.h>

#include <QtCore/QTemporaryFile>

using namespace MoleQueue;

class FileSpecificationTest : public QObject
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

  void ctorFromJsonObject();
  void ctorFromPath();
  void ctorFromFileNameAndContents();
  void ctorFromFile();
  void ctorCopy();
  void assignment();

  void format();
  void isValid();
  void toJsonObject();
  void fileExists();
  void writeFile();
  void filename();
  void contents();
  void filepath();
  void fileHasExtension();
  void fileBaseName();
  void fileExtension();

};

QString FileSpecificationTest::readReferenceString(const QString &filename_)
{
  QString realFilename = MoleQueue_TESTDATA_DIR + filename_;
  QFile refFile (realFilename);
  if (!refFile.open(QFile::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot access reference file" << realFilename;
    return "";
  }
  QString result = QString(refFile.readAll());
  refFile.close();
  return result;
}

void FileSpecificationTest::initTestCase()
{
}

void FileSpecificationTest::cleanupTestCase()
{
}

void FileSpecificationTest::init()
{
}

void FileSpecificationTest::cleanup()
{
}

void FileSpecificationTest::ctorFromJsonObject()
{
  QJsonObject json;
  json.insert("path", QString("/some/path/to/a/file.ext"));

  FileSpecification pathSpec(json);
  QString ref = readReferenceString("filespec-ref/path.json");
  QCOMPARE(QString(pathSpec.toJson()), ref);
}

void FileSpecificationTest::ctorFromPath()
{
  FileSpecification pathSpec(QString("/some/path/to/a/file.ext"));
  QString ref = readReferenceString("filespec-ref/path.json");
  QCOMPARE(QString(pathSpec.toJson()), ref);
}

void FileSpecificationTest::ctorFromFileNameAndContents()
{
  FileSpecification contSpec(QString("file.ext"), QString("I'm input file text!\n"));
  QString ref = readReferenceString("filespec-ref/contents.json");
  QCOMPARE(QString(contSpec.toJson()), ref);
}

void FileSpecificationTest::ctorFromFile()
{
  QTemporaryFile file;
  QVERIFY(file.open());
  file.setTextModeEnabled(true);

  QByteArray content("I'm input file text!!\n");
  file.write(content);

  file.close();

  FileSpecification spec(&file, FileSpecification::PathFileSpecification);
  QCOMPARE(spec.format(), FileSpecification::PathFileSpecification);
  QCOMPARE(spec.filepath(), QFileInfo(file).absoluteFilePath());

  spec = FileSpecification(&file, FileSpecification::ContentsFileSpecification);
  QCOMPARE(spec.format(), FileSpecification::ContentsFileSpecification);
  QCOMPARE(spec.filename(), QFileInfo(file).fileName());
  QCOMPARE(spec.contents(), QString(content));
}

void FileSpecificationTest::ctorCopy()
{
  FileSpecification spec1(QString("/path/to/some/file.ext"));
  FileSpecification spec2(spec1);

  QCOMPARE(spec1.toJson(), spec2.toJson());
}

void FileSpecificationTest::assignment()
{
  FileSpecification spec1(QString("/path/to/some/file.ext"));
  FileSpecification spec2;
  spec2 = spec1;

  QCOMPARE(spec1.toJson(), spec2.toJson());
}

void FileSpecificationTest::format()
{
  FileSpecification pathSpec(QString("/some/path/to/a/file.ext"));
  QCOMPARE(pathSpec.format(), FileSpecification::PathFileSpecification);

  FileSpecification contSpec(QString("file.ext"), QString("I'm input file text!\n"));
  QCOMPARE(contSpec.format(), FileSpecification::ContentsFileSpecification);

  QJsonObject json;

  FileSpecification inv1(json);
  QCOMPARE(inv1.format(), FileSpecification::InvalidFileSpecification);

  json.insert("notARealKey", QLatin1String("Bad value!"));
  FileSpecification inv2(json);
  QCOMPARE(inv2.format(), FileSpecification::InvalidFileSpecification);

  // filename, but no contents
  json.insert("filename", QLatin1String("Bad value!"));
  FileSpecification inv3(json);
  QCOMPARE(inv3.format(), FileSpecification::InvalidFileSpecification);

  FileSpecification inv4;
  QCOMPARE(inv4.format(), FileSpecification::InvalidFileSpecification);
}

void FileSpecificationTest::isValid()
{
  FileSpecification pathSpec(QString("/some/path/to/a/file.ext"));
  QVERIFY(pathSpec.isValid());

  FileSpecification contSpec(QString("file.ext"), QString("I'm input file text!\n"));
  QVERIFY(contSpec.isValid());

  QJsonObject json;
  FileSpecification inv(json);
  QVERIFY(!inv.isValid());
}

void FileSpecificationTest::toJsonObject()
{
  FileSpecification pathSpec(QString("/some/path/to/a/file.ext"));
  QJsonObject pathJson = pathSpec.toJsonObject();
  QCOMPARE(pathJson["path"].toString(),
           QString("/some/path/to/a/file.ext"));

  FileSpecification contSpec(QString("file.ext"), QString("I'm input file text!\n"));
  QJsonObject contJson = contSpec.toJsonObject();
  QCOMPARE(contJson["filename"].toString(),
           QString("file.ext"));
  QCOMPARE(contJson["contents"].toString(),
           QString("I'm input file text!\n"));
}

void FileSpecificationTest::fileExists()
{
  QTemporaryFile file;
  // filename isn't generated until open is called.
  file.open();

  FileSpecification spec(&file, FileSpecification::PathFileSpecification);
  QVERIFY(spec.fileExists());

  /// Always returns false for contents, since no path is known.
  spec = FileSpecification(&file, FileSpecification::ContentsFileSpecification);
  QVERIFY(!spec.fileExists());

  file.close();
}

void FileSpecificationTest::writeFile()
{
  QTemporaryFile file;
  // filename isn't available until open is called.
  QVERIFY(file.open());
  file.setTextModeEnabled(true);

  QString content("I'm sample input file contents!\n");
  FileSpecification spec(file.fileName(), content);

  spec.writeFile(QFileInfo(file).dir());
  QCOMPARE(QString(file.readAll()), content);
  file.close();
}

void FileSpecificationTest::filename()
{
  FileSpecification contSpec("file.ext", "contents\n");
  QCOMPARE(contSpec.filename(), QString("file.ext"));

  FileSpecification pathSpec(QString("/path/to/some/file.ext"));
  QCOMPARE(pathSpec.filename(), QString("file.ext"));
}

void FileSpecificationTest::contents()
{
  QTemporaryFile file;
  // filename isn't available until open is called.
  file.open();
  QString content("I'm sample input file contents!\n");
  FileSpecification spec(file.fileName(), content);
  QCOMPARE(spec.contents(), content);

  spec.writeFile(QFileInfo(file).dir());
  QCOMPARE(spec.contents(), content);
  file.close();
}

void FileSpecificationTest::filepath()
{
  FileSpecification pathSpec(QString("/path/to/some/file.ext"));
#ifdef _WIN32
  QCOMPARE(pathSpec.filepath(), QString("C:/path/to/some/file.ext"));
#else
  QCOMPARE(pathSpec.filepath(), QString("/path/to/some/file.ext"));
#endif

  FileSpecification contSpec("file.ext", "contents\n");
  QVERIFY(contSpec.filepath().isNull());
}

void FileSpecificationTest::fileHasExtension()
{
  FileSpecification pathSpec(QString("/path/to/some/file.ext"));
  QVERIFY(pathSpec.fileHasExtension());
  pathSpec = FileSpecification(QString("/path/to/some/file"));
  QVERIFY(!pathSpec.fileHasExtension());

  FileSpecification contSpec("file.ext", "contents\n");
  QVERIFY(contSpec.fileHasExtension());
  contSpec = FileSpecification("file", "contents\n");
  QVERIFY(!contSpec.fileHasExtension());
}

void FileSpecificationTest::fileBaseName()
{
  FileSpecification pathSpec(QString("/path/to/some/file.ext"));
  QCOMPARE(pathSpec.fileBaseName(), QString("file"));
  pathSpec = FileSpecification(QString("/path/to/some/file"));
  QCOMPARE(pathSpec.fileBaseName(), QString("file"));

  FileSpecification contSpec("file.ext", "contents\n");
  QCOMPARE(contSpec.fileBaseName(), QString("file"));
  contSpec = FileSpecification("file", "contents\n");
  QCOMPARE(contSpec.fileBaseName(), QString("file"));
}

void FileSpecificationTest::fileExtension()
{
  FileSpecification pathSpec(QString("/path/to/some/file.ext"));
  QCOMPARE(pathSpec.fileExtension(), QString("ext"));
  pathSpec = FileSpecification(QString("/path/to/some/file"));
  QVERIFY(pathSpec.fileExtension().isNull());

  FileSpecification contSpec("file.ext", "contents\n");
  QCOMPARE(contSpec.fileExtension(), QString("ext"));
  contSpec = FileSpecification("file", "contents\n");
  QVERIFY(contSpec.fileExtension().isNull());
}

QTEST_MAIN(FileSpecificationTest)

#include "filespecificationtest.moc"
