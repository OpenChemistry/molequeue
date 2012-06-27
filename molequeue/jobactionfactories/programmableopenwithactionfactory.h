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

#ifndef PROGRAMMABLEOPENWITHACTIONFACTORY_H
#define PROGRAMMABLEOPENWITHACTIONFACTORY_H

#include "openwithactionfactory.h"

#include <QtCore/QList>
#include <QtCore/QRegExp>

namespace MoleQueue
{

/**
 * @class ProgrammableOpenWithActionFactory programmableopenwithactionfactory.h
 * <molequeue/jobactionfactories/programmableopenwithactionfactory.h>
 * @brief OpenWithActionFactory subclass that is configured at runtime.
 * @author David C. Lonie
 *
 * ProgrammableOpenWithActionFactory is a concrete subclass of
 * OpenWithActionFactory that is configurable at runtime to launch one or more
 * instances of an application with a job's output file as an argument.
 *
 * Validity of Jobs is determined by comparing a Job's output filename against
 * one or more QRegExp objects, set by setRecognizedFilePatterns.
 *
 * To use this class, supply the name of the target application's executable
 * with setExecutableName and a list of QRegExp objects that define valid output
 * filenames.
 *
 * The OpenWithManagerDialog provides a user-friendly GUI element for
 * adding, removing, and configuring the set of
 * ProgrammableOpenWithActionFactory objects in the ActionFactoryManager.
 */
class ProgrammableOpenWithActionFactory : public OpenWithActionFactory
{
  Q_OBJECT
public:
  ProgrammableOpenWithActionFactory();
  ProgrammableOpenWithActionFactory(const ProgrammableOpenWithActionFactory &);
  ProgrammableOpenWithActionFactory &operator=(const ProgrammableOpenWithActionFactory &);
  virtual ~ProgrammableOpenWithActionFactory();

  virtual void readSettings(QSettings &settings);
  virtual void writeSettings(QSettings &settings) const;

  /**
   * Set the name of the target application's executable.
   */
  void setExecutableName(const QString &name) { m_executableName = name; }

  /**
   * Set a list of QRegExp objects that define a set of valid job output
   * filenames.
   */
  void setRecognizedFilePatterns(const QList<QRegExp> &patterns);

  /**
   * Return a list of the QRegExp objects used to identify valid jobs.
   */
  QList<QRegExp> recognizedFilePatterns() const;

  /**
   * Return a reference to the internal list of QRegExp objects. Used by
   * the OpenWithManagerDialog to configure the factory.
   */
  QList<QRegExp> & recognizedFilePatternsRef();

  /**
   * Compare the output file from Job against the list of recognized file
   * patterns. Returns true if a match is found, false otherwise.
   */
  virtual bool isValidForJob(const Job *job) const;

protected:
  QList<QRegExp> m_recognizedFilePatterns;
};

} // end namespace MoleQueue

#endif // PROGRAMMABLEOPENWITHACTIONFACTORY_H
