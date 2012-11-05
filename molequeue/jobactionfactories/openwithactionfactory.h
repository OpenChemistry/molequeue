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

#include "molequeueglobal.h"

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
 * See ProgrammableOpenWithActionFactory for an example.
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

  /** Reimplemented from base class. */
  void clearJobs();

  /**
   * Get the absolute path to the executable. May be empty until
   * actionTriggered is called.
   */
  QString executableFilePath() const { return m_executableFilePath; }

  /**
   * Get the name of the executable.
   */
  QString executableName() const { return m_executableName; }

  /// Reimplemented from base class.
  bool useMenu() const;

  /// Reimplemented from base class.
  QString menuText() const;

  /**
   * Return a list of new QActions. The QActions will be preconfigured and must
   * be deleted by the caller.
   *
   * The default implementation returns a QAction for each filename that the
   * executable can handle. The text of the action will be the filename, and
   * will be placed in a submenu with text
   * "Open ['job description]' in [executable name]..."
   *
   * The QVariant data member of the action is set to the list of accepted job.
   *
   * The action's "filename" property contains the absolute path to the file.
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
   * -# Extract the Job from the sender's QVariant data.
   * -# Extract the filename from the sender's dynamic properties.
   * -# Set m_executableFilePath using the following methods, until one finds a
   *    file that both exists and is executable:
   *    -# Check QSettings "ActionFactory/OpenWith/path/[executable name]" path
   *       for the last used path.
   *    -# Search the locations in the PATH environment variable for the
   *       executable.
   *    -# Ask the user to specify the location of the executable.
   *  -# Call "[m_executableFilePath] [filename]" via
   *     QProcess::startDetached().
   */
  virtual void actionTriggered();

protected:
  /// Return true and set m_executablePath if executable @a exec found in $PATH.
  virtual bool searchPathForExecutable(const QString &exec);

  QString m_executableFilePath;
  QString m_executableName;
  // display text, absolute file path
  mutable QMap<QString, QString> m_filenames;
  QString m_menuText;
};

} // end namespace MoleQueue

#endif // OPENWITHACTIONFACTORY_H
