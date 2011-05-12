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

#include <QtCore/QString>
#include <QtCore/QMap>

namespace MoleQueue {

class Queue;

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

class Program
{

public:
  explicit Program(Queue *queue = 0);
  ~Program();

  /** Copy constructor. */
  Program(const Program &other);

  /**
   * @enum Various valid job statuses.
   */
  enum Status {
    UNDEFINED = 0,
    QUEUED,
    REMOTEQUEUED,
    RUNNING,
    COMPLETE,
    FAILED
  };

  /**
   * Set the name of the program. This is the name that will often show up in
   * the GUI, and many common names such as GAMESS, GAMESS-UK, Gaussian,
   * MolPro etc are used by GUIs such as Avogadro with its input generator
   * dialogs to match up input files to programs.
   */
  void setName(const QString &name) { m_name = name; }

  /** Get the name of the program. Often used by GUIs etc. */
  QString name() const { return m_name; }

  /**
   * Set the title of the job. This is the title that will show up in job list
   * in the GUI
   */
  void setTitle(const QString &title) { m_title = title; }

  /** Get the name of the program. Often used by GUIs etc. */
  QString title() const { return m_title; }

  /**
   * Will the program be run directly, or via an execution script?
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

  /**
   * Return a string with a list of keyword and replacement value pairs.
   * Mainly useful for debugging purposes.
   */
  QString replacementList() const;

  /**
   * Set the Queue that the Program belongs to, this is effectively the parent.
   */
  void setQueue(Queue *queue) { m_queue = queue; }

  /**
   * Get the queue that the program belongs to.
   */
  Queue * queue() { return m_queue; }
  const Queue * queue() const { return m_queue; }

  /**
   * Get the name of the queue that the program belongs to.
   */
  QString queueName() const;

  /**
   * \return The working directory (usually relative to home directory).
   */
  QString workingDirectory() const { return m_workingDirectory; }

  /**
   * Set the working directory (usually relative to the home directory) to run
   * the program.
   * \param dir The working directory to use.
   */
  void setWorkingDirectory(const QString &dir);

  /**
   * \return The input file that will be used when running the job.
   */
  QString inputFile() const { return m_inputFile; }

  /**
   * Set the input file to use for the job.
   * \param file The input file path.
   */
  void setInputFile(const QString &file);

  /**
   * Set the current status of the job.
   */
  void setStatus(Status status) { m_status = status; }

  /**
   * Get the current status of the job.
   */
  Status status() const { return m_status; }

  /**
   * Get a string describing the current status of the job.
   */
  QString statusString() const;

protected:
  /** The name of the program. This is normally used to describe what programs
   * have been configured for each queue.
   */
  QString m_name;

  /** The title of the job, if set. */
  QString m_title;

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

  /** The working directory (usually relative to home directory) to run code. */
  QString m_workingDirectory;

  /** Path to the input file. */
  QString m_inputFile;

  /** The current status of the job. */
  Status m_status;

  /** The Queue that the Program belongs to/is being run by. */
  Queue *m_queue;

};

} // End namespace

#endif // PROGRAM_H
