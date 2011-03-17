/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef PROGRAM_H
#define PROGRAM_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>

namespace MoleQueue {

/**
 * Class to represent a computer program. Embodies how to execute the program,
 * possibly logic to set the number of cores (if specified on the command line)
 * as well as letting the queue know how many cores the job requires. Once the
 * program is finished, indicates the expected output files that should be
 * produced after a successful run.
 *
 * The Program object is usually specific to the queue, but may be identical
 * if the executable, and options etc remain unchanged.
 */

class Program : public QObject
{
  Q_OBJECT

public:
  explicit Program(QObject *parent = 0);
  ~Program();

  /**
   * Will the program be run directly, or via an execution script.
   * Note that many queuing systems require that a job specification be
   * written, but local jobs might avoid this step for some codes.
   */
  bool isRunDirect() const { return m_runDirect; }

  /** Set whether the program should be run directly, or via a shell script. */
  void setRunDirect(bool isDirect) { m_runDirect = isDirect; }

  /**
   * The unexpanded template for running the code. This should be a generic
   * version that has at a minimum the standard replacement for the input file,
   * $$inputFile$$, and optionally the number of cores, $$nCPU$$.
   */
  QString runTemplate() const { return m_runTemplate; }

  /**
   * The expanded form of the run template, with keyword substitutions.
   */
  QString expandedRunTemplate() const;

  /**
   * Set the run template for the program. This should be a generic
   * version that has at a minimum the standard replacement for the input file,
   * $$inputFile$$, and optionally the number of cores, $$nCPU$$.
   */
  void setRunTemplate(const QString &runTemplate) { m_runTemplate = runTemplate; }

  /**
   * \return The keyword delimiter, defaults to $$. Should be at either side
   * of all keywords.
   */
  QString delimiter() const { return m_delimiter; }

  /**
   * Set the keyword delimiter, defaults to $$. Should be at either side of all
   * keywords.
   * \param delimiter The delimiter to use.
   */
  void setDelimiter(const QString &delimiter) { m_delimiter = delimiter; }

  /**
   * Get the full list of string keywords and replacements.
   */
  QString replacement(const QString &keyword) const;

  /**
   * Set a keyword and replacement pair.
   */
  void setReplacement(const QString &keyword, const QString &value);

protected:
  /** Should the code be run directly, or via a shell script? */
  bool m_runDirect;

  /** Template for running the program. This will either be a direct run from
   * the command line, or a shell script template. There are several standard
   * replacements, such as $$nCPU$$, the delimiter can also be changed if
   * necessary.
   */
  QString m_runTemplate;

  /** The delimiter to be used at either side of keywords for replacement. */
  QString m_delimiter;

  /** List of keyword, replacement pairs. This can be used to build up
   * complex program input specifications for computational codes.
   */
  QMap<QString, QString> m_replacements;

};

} // End namespace

#endif // PROGRAM_H
