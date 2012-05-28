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

#ifndef QUEUE_H
#define QUEUE_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QMetaType>
#include <QtCore/QList>
#include <QtCore/QStringList>

#include "program.h"

class QSettings;

namespace MoleQueue {

class Job;

/**
 * Abstract queue, generally want QueueLocal, or QueueRemote derived classes
 * as they will provide the facilities required. The queue instances
 * themselves refer to the Program classes to actually run jobs with a
 * particular code.
 *
 * Some of the states are skipped for local jobs where there is no separate
 * queue manager such as SGE or PBS. This queue class is a simple FIFO approach
 * and it is assumed that the remote job management system will implement any
 * more sophisticated job management, balancing, etc.
 */

class Queue : public QObject
{
  Q_OBJECT
public:
  explicit Queue(const QString &queueName = "Undefined", QObject *parentObject = 0);
  ~Queue();

  /**
   * Set the name of the queue. This should be unique, and will be used in the
   * GUI to refer to this queue.
   */
  virtual void setName(const QString &newName) { m_name = newName; }

  /** Get the name of the queue. */
  QString name() const { return m_name; }

  /**
   * Returns the type of the queue as a string.
   */
  virtual QString typeName() const { return "Unknown"; }

  /**
   * Read settings for the queue, done early on at startup.
   */
  virtual void readSettings(const QSettings &settings);

  /**
   * Write settings for the queue, done just before closing the server.
   */
  virtual void writeSettings(QSettings &settings) const;

  /**
   * Returns a widget that can be used to configure the settings for the
   * queue.
   */
  virtual QWidget* settingsWidget() const;

  /**
   * Add a new program to the queue. Program names must be unique in each
   * queue, as they are used to specify which program will be used.
   * @param program The program to be added to the queue.
   * @param replace Defaults to false, if true replace any program with the
   * same name in this queue.
   * @return True on success, false on failure.
   */
  bool addProgram(Program *newProgram, bool replace = false);

  /**
   * Attempt to remove a program from the queue. The program name is used
   * as the criteria to decice which object to remove.
   * @param program The program to be removed from the queue.
   * @return True on success, false on failure.
   */
  bool removeProgram(Program *programToRemove);

  /**
   * Attempt to remove a program from the queue. The program name is used
   * as the criteria to decide which object to remove.
   * @param name The name of the program to be removed from the queue.
   * @return True on success, false on failure.
   */
  bool removeProgram(const QString &programName);

  /**
   * Retrieve the program object associated with the supplied name.
   * @param name The name of the program.
   * @return The Program object, a null pointer is returned if the
   * requested program is not in this queue.
   */
  Program* program(const QString &programName);

  /**
   * Clear all programs, useful if you would like ot reset a Queue.
   */
  void clearPrograms();

  /**
   * Retrieve a list of the programs that this Queue object has been configured
   * with. This can be used by interfaces to show available resources.
   */
  QStringList programs() const;

signals:
  void jobAdded(Job *job);
  void jobStateChanged(Job *job);
  void jobRemoved(Job *job);

public slots:
  /**
   * Submit a new job to the queue.
   * \param job The Program object to submit to the queue.
   * \return True on successful addition to the queue.
   */
  virtual bool submit(Job *job);

protected:
  QString m_name;
  QMap<QString, Program *> m_programs;
  QList<Job *> m_jobs;

  /** This stores the long running job number, largely used as a directory for
   * storage or staged files. This is often an offset applied to the locally
   * used index into the array of actual jobs stored in a running instance.
   * The first time MoleQueue is run for a new queue it will be zero, after that
   * it will keep a count of the total number of jobs run.
   *
   * This will require clean up passes/possible reset at some future time. The
   * data will live longer term in a database/permanent storage. Queues are only
   * intended for short term storage while processing jobs.
   */
  unsigned int m_jobIndexOffset;

};

} // End namespace

Q_DECLARE_METATYPE(MoleQueue::Queue*)
Q_DECLARE_METATYPE(const MoleQueue::Queue*)

#endif // QUEUE_H
