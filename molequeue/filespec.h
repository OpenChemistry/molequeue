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

#ifndef MOLEQUEUE_FILESPEC_H
#define MOLEQUEUE_FILESPEC_H

#include <QtCore/QVariantHash>

class QDir;
class QFile;

namespace MoleQueue
{
class FileSpecPrivate;

/**
 * @class FileSpec filespec.h <molequeue/filespec.h>
 * @brief Specify files for simplifying Client-Server communication.
 * @author David C. Lonie
 *
 * The FileSpec class converts between Qt and JsonCpp types to facilite file
 * manipulation during RPC communication. Files are stored as either a path
 * to the local file on disk, or a filename and content string.
 */
class FileSpec
{
public:
  /// Recognized internal formats for storing file data
  enum Format {
    /// Invalid format
    InvalidFileSpec = -1,
    /// Single "filepath" member pointing to a location on the filesystem
    PathFileSpec = 0,
    /// Filename and content strings.
    ContentsFileSpec
  };

  /// Creates an invalid FileSpec.
  FileSpec();

  /// Create a FileSpec using the members of the input QVariantHash.
  explicit FileSpec(const QVariantHash &hash);

  /// Create a FileSpec from the input absolute filepath.
  explicit FileSpec(const QString &path);

  /// Create a FileSpec from the filename and content strings.
  FileSpec(const QString &filename_, const QString &contents_);

  /// Create a FileSpec from the specified file using the indicated format
  FileSpec(QFile *file, Format format_ = PathFileSpec);

  /// Copy a FileSpec
  FileSpec(const FileSpec &other);

  /// Copy a FileSpec
  FileSpec & operator=(const FileSpec &other);

  /// Destroy the FileSpec
  ~FileSpec();

  /// @return The format of the FileSpec
  /// @see FileSpec::Format
  Format format() const;

  /// @return True if the FileSpec is formatted properly, false otherwise.
  bool isValid() const { return format() != InvalidFileSpec; }

  /// @return The FileSpec as a formatted JSON string.
  QString asJsonString() const;

  /// @return The FileSpec as a QVariantHash.
  QVariantHash asVariantHash() const;

  /// @return Whether or not the FileSpec refers to an existing file
  /// @note This will always be false if format() does not return
  /// ContentsFileSpec.
  bool fileExists() const;

  /// Write contents() to a file with @a filename_ in @a dir. If @a filename
  /// is not specified, filename() will be used instead (default).
  /// @return True if the file is successfully written.
  bool writeFile(const QDir &dir, const QString &filename_ = QString()) const;

  /// @return The filename (without path) of the FileSpec.
  QString filename() const;

  /// @return The contents of the file.
  QString contents() const;

  /// @return The filename (with path) of the FileSpec.
  /// @note This function only makes sense if format() is PathFileSpec. It will
  /// always return a null string otherwise.
  QString filepath() const;

  /// @return True if the filename has an extension ("file.ext"), false
  /// otherwise ("file")
  bool fileHasExtension() const;

  /// @return The filename without an extension.
  QString fileBaseName() const;

  /// @return The file extension, if any, or a null string.
  /// @see hasFileExtension
  QString fileExtension() const;

private:
  FileSpecPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(FileSpec)
};

} // namespace MoleQueue

#endif // MOLEQUEUE_FILESPEC_H
