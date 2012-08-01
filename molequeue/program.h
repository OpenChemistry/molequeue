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

#ifndef PROGRAM_H
#define PROGRAM_H

#include <QtCore/QObject>

#include <QtCore/QMap>
#include <QtCore/QMetaType>
#include <QtCore/QString>

class QSettings;

namespace MoleQueue
{
class Queue;
class QueueManager;
class Server;

/**
 * @class Program program.h <molequeue/program.h>
 * @brief A class defining interactions with an executable accessible by a Queue.
 * @author Marcus D. Hanwell, David C. Lonie
 *
 * The Program class describes an executable which runs a Job on a particular
 * Queue. Each Program object is unique to the Queue, and contains details
 * for running the executable, any arguments it needs, and the names of files
 * it reads/produces.
 */
class Program : public QObject
{
  Q_OBJECT
public:
  explicit Program(Queue *parentQueue = 0);
  Program(const Program &other);
  ~Program();
  Program &operator=(const Program &other);

  /// Enum used for various common styles of execution syntax
  enum LaunchSyntax {
    /// Use custom launch script
    CUSTOM = 0,
    /// Only run the executable, e.g. "vasp"
    PLAIN,
    /// Single argument is the name of the input file with extension, e.g.
    /// "mopac job.mop"
    INPUT_ARG,
    /// Single argument is the name of the input file without extension, e.g.
    /// "mopac job"
    INPUT_ARG_NO_EXT,
    /// Redirect input file to stdin and stdout to output file, e.g.
    /// "gulp < job.gin > job.got"
    REDIRECT,
    /// Input as argument, redirect stdout to output file, e.g.
    /// "gamess job.inp > job.out"
    INPUT_ARG_OUTPUT_REDIRECT,

    /// Use to get total number of syntax types.
    SYNTAX_COUNT
  };

  /// @return The parent Server
  Server *server() {return m_server;}
  /// @return The parent Server
  const Server *server() const {return m_server;}

  /// @return The parent QueueManager
  QueueManager *queueManager() {return m_queueManager;}
  /// @return The parent Server
  const QueueManager *queueManager() const {return m_queueManager;}

  /// @return The Queue that this Program belongs to.
  Queue * queue() { return m_queue; }
  /// @return The Queue that this Program belongs to.
  const Queue * queue() const { return m_queue; }

  /// @return The name of the Queue that this Program belongs to.
  QString queueName() const;

  /// @param settings QSettings object to write state to.
  void readSettings(QSettings &settings);
  /// @param settings QSettings object to read state from.
  void writeSettings(QSettings &settings) const;

  /// Import the program's configuration from the @a settings object.
  void importConfiguration(QSettings &importer);

  /// Export the program's configuration into the @a settings object.
  void exportConfiguration(QSettings &exporter) const;

  /**
   * Set the name of the program. This is the name that will show up in
   * the GUI, and many common names such as GAMESS, GAMESS-UK, Gaussian,
   * MolPro etc are used by GUIs such as Avogadro with its input generator
   * dialogs to match up input files to programs.
   * @param newName Name to use in GUIs
   */
  void setName(const QString &newName) { m_name = newName; }

  /// @return The name of the program. Often used by GUIs etc.
  QString name() const { return m_name; }

  void setExecutable(const QString &str) {m_executable = str;}
  QString executable() const {return m_executable;}

  void setUseExecutablePath(bool b) {m_useExecutablePath = b;}
  bool useExecutablePath() const {return m_useExecutablePath;}

  void setExecutablePath(const QString &str) {m_executablePath = str;}
  QString executablePath() const {return m_executablePath;}

  /// Arguments for executable -- do not include input file!
  void setArguments(const QString &str) {m_arguments = str;}
  QString arguments() const {return m_arguments;}

  void setInputFilename(const QString &str) {m_inputFilename = str;}
  QString inputFilename() const {return m_inputFilename;}
  QString inputFilenameNoExtension() const
  {
    return chopExtension(m_inputFilename);
  }

  void setOutputFilename(const QString &str) {m_outputFilename = str;}
  QString outputFilename() const {return m_outputFilename;}
  QString outputFilenameNoExtension() const
  {
    return chopExtension(m_outputFilename);
  }

  void setLaunchSyntax(LaunchSyntax s)
  {
    if (s >= SYNTAX_COUNT)
      return;
    m_launchSyntax = s;
  }
  LaunchSyntax launchSyntax() const {return m_launchSyntax;}

  void setCustomLaunchTemplate(const QString &str)
  {
    m_customLaunchTemplate = str;
  }
  QString customLaunchTemplate() const {return m_customLaunchTemplate;}

  /// @return Either the custom launch template or a default generated template,
  /// depending on the value of launchSyntax.
  QString launchTemplate() const;

  static QString generateFormattedExecutionString(
      const QString &executableName_, const QString &arguments_,
      const QString &inputFilename_, const QString &outputFilename_,
      const QString &executablePath_,
      bool useExecutablePath_, LaunchSyntax syntax_);

protected:

  /// Internal convenience function
  static QString chopExtension(const QString & str)
  {
    QString ret (str);
    int extensionIndex = ret.lastIndexOf(".");
    if (extensionIndex != -1)
      ret.truncate(extensionIndex);
    return ret;
  }

  /// The Queue that the Program belongs to/is being run by.
  Queue *m_queue;
  /// The QueueManager owning the Queue this Program belongs to.
  QueueManager *m_queueManager;
  /// The Server this program is associated with.
  Server *m_server;
  /// GUI-visible name
  QString m_name;
  /// Name of executable
  QString m_executable;
  /// Toggle inclusion of path to executable
  bool m_useExecutablePath;
  /// Path to executable
  QString m_executablePath;
  /// Executable arguments
  QString m_arguments;
  /// Input filename
  QString m_inputFilename;
  /// Output filename
  QString m_outputFilename;
  /// Launch syntax style
  LaunchSyntax m_launchSyntax;
  /// Bash/Shell/Queue script template used to launch program
  QString m_customLaunchTemplate;

};

} // End namespace

Q_DECLARE_METATYPE(MoleQueue::Program*)
Q_DECLARE_METATYPE(const MoleQueue::Program*)

#endif // PROGRAM_H
