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

#ifndef JOB_H
#define JOB_H

#include <QObject>

#include <QMap>

namespace MoleQueue {

class Queue;
class Program;

/**
 * Class to represent an execution of a Program.
 */
class Job : public QObject
{
  Q_OBJECT

public:
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

  /** Creates a new job. */
  explicit Job(const Program *program);

  /* Destroys the job object. */
  ~Job();

  /** Set the name of the job to \p name. */
  void setName(const QString &name);

  /** Returns the name of the job. */
  QString name() const;

  /** Sets the title of the job to \p title. */
  void setTitle(const QString &title);

  /** Returns the title for the job. */
  QString title() const;

  /**
   * Set the current status of the job.
   */
  void setStatus(Status status) { m_status = status; }

  /**
   * Get the current status of the job.
   */
  Status status() const { return m_status; }

  /** Returns the program that the job is a type of. */
  const Program* program() const;

  /** Returns the queue that the job is a member of. */
  const Queue* queue() const;

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
   * \return The input that will be used when running the job, if empty then
   * it is assumed that m_inputFile points to a valid input file.
   */
  QString input() const { return m_input; }

  /**
   * Set the input to use for the job. If this is set, it will be used in
   * preference to the inputFile, and will be written to disk.
   * \param file The input file path.
   */
  void setInput(const QString &input) { m_input = input; }

  /**
   * \return The full path of the output file that was produced. This is used
   * to open the output file in external programs.
   */
  QString outputFile() const { return m_outputFile; }

  /**
   * Set the output file to produced by the job.
   * \param file The input file path.
   */
  void setOutputFile(const QString &file);

  /**
   * Get a string describing the current status of the job.
   */
  QString statusString() const;

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
   * The expanded form of the run template, with keyword substitutions.
   */
  QString expandedRunTemplate() const;

private:
  /** The name of the job. */
  QString m_name;

  /** The title of the job. */
  QString m_title;

  /** The program that the job is a type of. */
  const Program* m_program;

  /** The current status of the job. */
  Status m_status;

  /** Path to the input file. */
  QString m_inputFile;

  /** The input file, if set this will be written and sent to the server. */
  QString m_input;

  /** Full path to the output file. */
  QString m_outputFile;

  /** The working directory (usually relative to home directory) to run code. */
  QString m_workingDirectory;

  /** List of keyword, replacement pairs. This can be used to build up
   * complex program input specifications for computational codes.
   */
  QMap<QString, QString> m_replacements;
};

} // end MoleQueue namespace

#endif // JOB_H
