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
 * Subclasses should, at minimum, set m_executable to the name of the
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

  void clearJobs();

  /**
   * The executable. May be either an absolute filepath, or an executable name
   * to be looked up in the system path.
   */
  QString executable() const { return m_executable; }

  bool useMenu() const;
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
   * The QVariant data member of the action is set a relevant Job object.
   *
   * The action's "filename" property contains the absolute path to the file.
   *
   * The QAction::triggered() signal is connected to the actionTriggered slot
   * of the factory. See that function's documentation for details of its
   * default implementation.
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
   * as an argument.
   */
  virtual void actionTriggered();

protected:
  QString m_name;
  QString m_executable;
  // display text, absolute file path
  mutable QMap<QString, QString> m_filenames;
  QString m_menuText;
};

} // end namespace MoleQueue

#endif // OPENWITHACTIONFACTORY_H
