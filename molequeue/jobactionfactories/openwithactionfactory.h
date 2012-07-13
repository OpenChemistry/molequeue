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

#ifndef OPENWITHACTIONFACTORY_H
#define OPENWITHACTIONFACTORY_H

#include "../jobactionfactory.h"

namespace MoleQueue
{

/**
 * @class OpenWithActionFactory openwithactionfactory.h
 * <molequeue/jobactionfactories/openwithactionfactory.h>
 * @brief JobActionFactory which opens job output in an external application.
 * @author David C. Lonie
 *
 * OpenWithActionFactory is an abstract subclass of JobActionFactory designed
 * to open job output with an external application.
 *
 * Subclasses should, at minimum, set m_executableName to the name of the
 * external application in the constructor and reimplement isValidForJob.
 */
class OpenWithActionFactory : public JobActionFactory
{
  Q_OBJECT
public:
  OpenWithActionFactory();
  OpenWithActionFactory(const OpenWithActionFactory &other);
  virtual ~OpenWithActionFactory();
  OpenWithActionFactory & operator=(const OpenWithActionFactory &other);

  /**
   * Read state from @a settings.
   */
  virtual void readSettings(QSettings &settings);

  /**
   * Write state to @a settings.
   */
  virtual void writeSettings(QSettings &settings) const;

  /**
   * Get the absolute path to the executable. May be empty until
   * actionTriggered is called.
   */
  QString executableFilePath() const { return m_executableFilePath; }

  /**
   * Get the name of the executable.
   */
  QString executableName() const { return m_executableName; }

  /**
   * Return true if the Job @a job can be opened by the application, false
   * otherwise.
   */
  virtual bool isValidForJob(const Job &job) const = 0;

  /**
   * Return a list of new QActions. The QActions will be preconfigured and must
   * be deleted by the caller.
   *
   * The default implementation returns a single QAction. The text will be:
   * - "Open '[job description]' in [executable name]..." if only one job was
   *   shown to addJobIfValid.
   * - "Open [m_jobs.size()] jobs in [executable name]..." if several jobs were
   *   shown to addJobIfValid and all of them were accepted.
   * - "Open [m_jobs.size()] of [m_attemptedJobAdditions] in
   *   [executable name]..." if several jobs were shown to addJobIfValid and
   *   some were rejected.
   *
   * The QVariant data member of the action is set to the list of accepted job.
   *
   * The QAction::triggered() signal is connected to the actionTriggered slot
   * of the factory. See that functions documentation for details its default
   * implementation.
   */
  virtual QList<QAction*> createActions();

  /** Reimplemented from JobActionFactory. Default value: 800. */
  virtual unsigned int usefulness() const;

protected slots:
  /**
   * This slot is connected to new the QAction::triggered() signal of new
   * actions returned from createActions. This function must only be called as
   * a response to the action's signal (i.e. sender() must be a QAction)
   * and the sender's QAction::data() method must return a QList<Job>
   * containing the jobs to be opened by the target application.
   *
   * The default implementation launches the external application for each job
   * in the sender's data list, with the absolute path to the job's output file
   * as an argument. This function attempts to determine and set
   * m_executableFilePath.
   *
   * An outline of this function's behavior:
   *
   * -# Check that sender() is a QAction
   * -# Extract the list of Job pointers from the sender's QVariant data.
   * -# Set m_executableFilePath using the following methods, until one finds a
   *    file that both exists and is executable:
   *    -# Check QSettings "ActionFactory/OpenWith/path/[executable name]" path
   *       for the last used path.
   *    -# Search the locations in the PATH environment variable for the
   *       executable.
   *    -# Ask the user to specify the location of the executable.
   * -# For each job in the sender's data list:
   *    -# Check that the filename (as determined by Job::outputDirectory() and
   *       Program::outputFilename()) refers to an existing file.
   *       -# If the file does not exist or the job's Program is no longer
   *          valid, ask the user which file in Job::outputDirectory() to open.
   *    -# Call "[m_executableFilePath] [absolute path to output file]" via
   *       QProcess::startDetached().
   */
  virtual void actionTriggered();

protected:
  /// Return true and set m_executablePath if executable @a exec found in $PATH.
  virtual bool searchPathForExecutable(const QString &exec);

  QString m_executableFilePath;
  QString m_executableName;
};

} // end namespace MoleQueue

#endif // OPENWITHACTIONFACTORY_H
