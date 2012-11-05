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

#include "programmableopenwithactionfactory.h"

#include "../job.h"
#include "../program.h"
#include "../queue.h"
#include "../queuemanager.h"
#include "../server.h"

#include <QtCore/QDir>

namespace MoleQueue
{


ProgrammableOpenWithActionFactory::ProgrammableOpenWithActionFactory()
  : OpenWithActionFactory()
{
  m_flags |= JobActionFactory::ContextItem |
      JobActionFactory::ProgrammableOpenWith;
}

ProgrammableOpenWithActionFactory::ProgrammableOpenWithActionFactory(
    const ProgrammableOpenWithActionFactory &other)
  : OpenWithActionFactory(other),
    m_recognizedFilePatterns(other.m_recognizedFilePatterns)
{
}

ProgrammableOpenWithActionFactory &ProgrammableOpenWithActionFactory::operator=(
    const ProgrammableOpenWithActionFactory &other)
{
  OpenWithActionFactory::operator=(other);
  m_recognizedFilePatterns = other.recognizedFilePatterns();
  return *this;
}

ProgrammableOpenWithActionFactory::~ProgrammableOpenWithActionFactory()
{
}

void ProgrammableOpenWithActionFactory::readSettings(QSettings &settings)
{
  int numPatterns = settings.beginReadArray("recognizedFilePatterns");

  for (int i = 0; i < numPatterns; ++i) {
    settings.setArrayIndex(i);
    m_recognizedFilePatterns << settings.value("regexp").toRegExp();
  }

  settings.endArray();

  OpenWithActionFactory::readSettings(settings);
}

void ProgrammableOpenWithActionFactory::writeSettings(QSettings &settings) const
{
  settings.beginWriteArray("recognizedFilePatterns",
                           m_recognizedFilePatterns.size());

  for (int i = 0; i < m_recognizedFilePatterns.size(); ++i) {
    settings.setArrayIndex(i);
    settings.setValue("regexp", m_recognizedFilePatterns[i]);
  }

  settings.endArray();

  OpenWithActionFactory::writeSettings(settings);
}

bool ProgrammableOpenWithActionFactory::isValidForJob(const Job &job) const
{
  if (!job.isValid())
    return false;

  QDir directory = job.jobState() == Finished ? QDir(job.outputDirectory())
                                              : QDir(job.localWorkingDirectory());

  return scanDirectoryForRecognizedFiles(directory, directory);
}

bool ProgrammableOpenWithActionFactory::scanDirectoryForRecognizedFiles(
    const QDir &baseDir, const QDir &dir) const
{
  bool result = false;

  // Recursively check subdirectories
  QStringList subDirs(dir.entryList(QDir::Dirs | QDir::Readable |
                                    QDir::NoDotAndDotDot, QDir::Name));
  foreach (const QString &subDir, subDirs) {
    if (scanDirectoryForRecognizedFiles(baseDir,
                                        QDir(dir.absoluteFilePath(subDir))))
      result = true;
  }

  QStringList entries(dir.entryList(QDir::Files | QDir::Readable,
                                    QDir::Name));

  foreach (const QString &filename, entries) {
    foreach (const QRegExp &regexp, m_recognizedFilePatterns) {
      if (regexp.indexIn(filename) >= 0) {
        result = true;
        m_filenames.insert(
              baseDir.relativeFilePath(dir.absoluteFilePath(filename)),
              dir.absoluteFilePath(filename));
      }
    }
  }

  return result;
}

void ProgrammableOpenWithActionFactory::setRecognizedFilePatterns(
    const QList<QRegExp> &patterns)
{
  m_recognizedFilePatterns = patterns;
}

QList<QRegExp> ProgrammableOpenWithActionFactory::recognizedFilePatterns() const
{
  return m_recognizedFilePatterns;
}

QList<QRegExp> & ProgrammableOpenWithActionFactory::recognizedFilePatternsRef()
{
  return m_recognizedFilePatterns;
}

} // end namespace MoleQueue
