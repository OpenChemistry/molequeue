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

#include "filespec.h"

#include "logger.h"
#include "qtjson.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <json/json.h>

#include <string>

namespace MoleQueue
{

class FileSpecPrivate
{
public:
  Json::Value json;
};

FileSpec::FileSpec()
  : d_ptr(new FileSpecPrivate())
{
}

FileSpec::FileSpec(const QVariantHash &hash)
  : d_ptr(new FileSpecPrivate())
{
  Q_D(FileSpec);
  d->json = QtJson::toJson(hash);
}

FileSpec::FileSpec(const QString &path)
  : d_ptr(new FileSpecPrivate())
{
  Q_D(FileSpec);
  d->json = Json::Value(Json::objectValue);
  d->json["filepath"] = path.toStdString();
}

FileSpec::FileSpec(const QString &filename_, const QString &contents_)
  : d_ptr(new FileSpecPrivate())
{
  Q_D(FileSpec);
  d->json = Json::Value(Json::objectValue);
  d->json["filename"] = filename_.toStdString();
  d->json["contents"] = contents_.toStdString();
}

FileSpec::FileSpec(QFile *file, FileSpec::Format format_)
  : d_ptr(new FileSpecPrivate())
{
  Q_D(FileSpec);
  switch (format_) {
  case PathFileSpec:
    d->json = Json::Value(Json::objectValue);
    d->json["filepath"] =
        QFileInfo(*file).absoluteFilePath().toStdString();
    break;
  case ContentsFileSpec: {
    d->json = Json::Value(Json::objectValue);
    d->json["filename"] = QFileInfo(*file).fileName().toStdString();
    if (!file->open(QFile::ReadOnly | QFile::Text)) {
      Logger::logError(Logger::tr("Error opening file for read: '%1'")
                       .arg(file->fileName()));
      return;
    }
    d->json["contents"] = file->readAll().data();
    file->close();
    break;
  }
  case InvalidFileSpec:
    Logger::logDebugMessage(Logger::tr("Cannot convert file to invalid file "
                                       "spec! (%1)").arg(file->fileName()));
    break;
  default:
    Logger::logDebugMessage(Logger::tr("Unknown filespec format (%1) for file "
                                       "'%2'").arg(format_).arg(file->fileName()));
    break;
  }
}

FileSpec::FileSpec(const FileSpec &other)
  : d_ptr(new FileSpecPrivate())
{
  Q_D(FileSpec);
  d->json = other.d_ptr->json;
}

FileSpec &FileSpec::operator=(const FileSpec &other)
{
  Q_D(FileSpec);
  d->json = other.d_ptr->json;
  return *this;
}

FileSpec::~FileSpec()
{
  delete d_ptr;
}

FileSpec::Format FileSpec::format() const
{
  Q_D(const FileSpec);
  if (d->json.isObject()) {
    if (d->json.isMember("filepath")) {
      return PathFileSpec;
    }
    else if (d->json.isMember("filename") &&
             d->json.isMember("contents")) {
      return ContentsFileSpec;
    }
  }

  return InvalidFileSpec;
}

QString FileSpec::asJsonString() const
{
  Q_D(const FileSpec);
  Json::StyledWriter writer;
  return QString::fromStdString(writer.write(d->json));
}

QVariantHash FileSpec::asVariantHash() const
{
  Q_D(const FileSpec);
  if (d->json.isObject())
    return QtJson::toVariant(d->json).toHash();
  return QVariantHash();
}

bool FileSpec::fileExists() const
{
  Q_D(const FileSpec);
  if (format() == PathFileSpec) {
    const char *path = d->json["filepath"].asCString();
    bool result = QFile::exists(path);
    return result;
//    return QFile::exists(d->json["filepath"].asCString());
  }

  return false;
}

bool FileSpec::writeFile(const QDir &dir, const QString &filename_) const
{
  switch (format()) {
  default:
  case InvalidFileSpec:
    return false;
  case PathFileSpec:
  case ContentsFileSpec: {
    QString path = dir.absoluteFilePath(filename_.isNull() ? filename()
                                                           : filename_);
    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
      return false;
    file.write(contents().toLocal8Bit());
    file.close();

    return true;
  }
  }
}

QString FileSpec::filename() const
{
  Q_D(const FileSpec);
  switch (format()) {
  default:
  case InvalidFileSpec:
    Logger::logDebugMessage(Logger::tr("Cannot extract filename from invalid "
                                       "filespec\n%1").arg(asJsonString()));
    return QString();
  case PathFileSpec:
    return QFileInfo(d->json["filepath"].asCString()).fileName();
  case ContentsFileSpec:
    return QFileInfo(d->json["filename"].asCString()).fileName();
  }
}

QString FileSpec::contents() const
{
  Q_D(const FileSpec);
  switch (format()) {
  default:
  case InvalidFileSpec:
    Logger::logWarning(Logger::tr("Cannot read contents of invalid filespec:"
                                  "\n%1").arg(asJsonString()));
    return QString();
  case PathFileSpec: {
    QFile file(filepath());
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      Logger::logError(Logger::tr("Error opening file for read: '%1'")
                       .arg(file.fileName()));
      return QString();
    }
    QString result = file.readAll();
    file.close();
    return result;
  }
  case ContentsFileSpec:
    return QString(d->json["contents"].asCString());
  }

}

QString FileSpec::filepath() const
{
  Q_D(const FileSpec);
  if (format() == PathFileSpec)
    return QFileInfo(d->json["filepath"].asCString()).absoluteFilePath();
  return QString();
}

bool FileSpec::fileHasExtension() const
{
  return !QFileInfo(filename()).suffix().isEmpty();
}

QString FileSpec::fileBaseName() const
{
  return QFileInfo(filename()).baseName();
}

QString FileSpec::fileExtension() const
{
  return QFileInfo(filename()).suffix();
}

} // namespace MoleQueue
