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

#include "filespecification.h"

#include "logger.h"
#include "qtjson.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <json/json.h>

#include <string>

namespace MoleQueue
{

class FileSpecificationPrivate
{
public:
  Json::Value json;
};

FileSpecification::FileSpecification()
  : d_ptr(new FileSpecificationPrivate())
{
}

FileSpecification::FileSpecification(const QVariantHash &hash)
  : d_ptr(new FileSpecificationPrivate())
{
  Q_D(FileSpecification);
  d->json = QtJson::toJson(hash);
}

FileSpecification::FileSpecification(const QString &path)
  : d_ptr(new FileSpecificationPrivate())
{
  Q_D(FileSpecification);
  d->json = Json::Value(Json::objectValue);
  d->json["filepath"] = path.toStdString();
}

FileSpecification::FileSpecification(const QString &filename_, const QString &contents_)
  : d_ptr(new FileSpecificationPrivate())
{
  Q_D(FileSpecification);
  d->json = Json::Value(Json::objectValue);
  d->json["filename"] = filename_.toStdString();
  d->json["contents"] = contents_.toStdString();
}

FileSpecification::FileSpecification(QFile *file, FileSpecification::Format format_)
  : d_ptr(new FileSpecificationPrivate())
{
  Q_D(FileSpecification);
  switch (format_) {
  case PathFileSpecification:
    d->json = Json::Value(Json::objectValue);
    d->json["filepath"] =
        QFileInfo(*file).absoluteFilePath().toStdString();
    break;
  case ContentsFileSpecification: {
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
  case InvalidFileSpecification:
    Logger::logDebugMessage(Logger::tr("Cannot convert file to invalid file "
                                       "spec! (%1)").arg(file->fileName()));
    break;
  default:
    Logger::logDebugMessage(Logger::tr("Unknown filespec format (%1) for file "
                                       "'%2'").arg(format_).arg(file->fileName()));
    break;
  }
}

FileSpecification::FileSpecification(const FileSpecification &other)
  : d_ptr(new FileSpecificationPrivate())
{
  Q_D(FileSpecification);
  d->json = other.d_ptr->json;
}

FileSpecification &FileSpecification::operator=(const FileSpecification &other)
{
  Q_D(FileSpecification);
  d->json = other.d_ptr->json;
  return *this;
}

FileSpecification::~FileSpecification()
{
  delete d_ptr;
}

FileSpecification::Format FileSpecification::format() const
{
  Q_D(const FileSpecification);
  if (d->json.isObject()) {
    if (d->json.isMember("filepath")) {
      return PathFileSpecification;
    }
    else if (d->json.isMember("filename") &&
             d->json.isMember("contents")) {
      return ContentsFileSpecification;
    }
  }

  return InvalidFileSpecification;
}

QString FileSpecification::asJsonString() const
{
  Q_D(const FileSpecification);
  Json::StyledWriter writer;
  return QString::fromStdString(writer.write(d->json));
}

QVariantHash FileSpecification::asVariantHash() const
{
  Q_D(const FileSpecification);
  if (d->json.isObject())
    return QtJson::toVariant(d->json).toHash();
  return QVariantHash();
}

bool FileSpecification::fileExists() const
{
  Q_D(const FileSpecification);
  if (format() == PathFileSpecification) {
    const char *path = d->json["filepath"].asCString();
    bool result = QFile::exists(path);
    return result;
//    return QFile::exists(d->json["filepath"].asCString());
  }

  return false;
}

bool FileSpecification::writeFile(const QDir &dir, const QString &filename_) const
{
  switch (format()) {
  default:
  case InvalidFileSpecification:
    return false;
  case PathFileSpecification:
  case ContentsFileSpecification: {
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

QString FileSpecification::filename() const
{
  Q_D(const FileSpecification);
  switch (format()) {
  default:
  case InvalidFileSpecification:
    Logger::logDebugMessage(Logger::tr("Cannot extract filename from invalid "
                                       "filespec\n%1").arg(asJsonString()));
    return QString();
  case PathFileSpecification:
    return QFileInfo(d->json["filepath"].asCString()).fileName();
  case ContentsFileSpecification:
    return QFileInfo(d->json["filename"].asCString()).fileName();
  }
}

QString FileSpecification::contents() const
{
  Q_D(const FileSpecification);
  switch (format()) {
  default:
  case InvalidFileSpecification:
    Logger::logWarning(Logger::tr("Cannot read contents of invalid filespec:"
                                  "\n%1").arg(asJsonString()));
    return QString();
  case PathFileSpecification: {
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
  case ContentsFileSpecification:
    return QString(d->json["contents"].asCString());
  }

}

QString FileSpecification::filepath() const
{
  Q_D(const FileSpecification);
  if (format() == PathFileSpecification)
    return QFileInfo(d->json["filepath"].asCString()).absoluteFilePath();
  return QString();
}

bool FileSpecification::fileHasExtension() const
{
  return !QFileInfo(filename()).suffix().isEmpty();
}

QString FileSpecification::fileBaseName() const
{
  return QFileInfo(filename()).baseName();
}

QString FileSpecification::fileExtension() const
{
  return QFileInfo(filename()).suffix();
}

} // namespace MoleQueue
