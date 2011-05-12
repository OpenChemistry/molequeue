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

#ifndef Connection_H
#define Connection_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtNetwork/QLocalSocket>

#include "program.h"

namespace MoleQueue {

/**
 * A connection to another program (usually a client submitting jobs).
 */

class Connection : public QObject
{
  Q_OBJECT
public:
  explicit Connection(QLocalSocket *socket, QObject *parent = 0);
  ~Connection();

  /**
   * An enum to track the current state of communication with the client.
   */
  enum State {
    IDLE,
    INPUT_FILE,
    OUTPUT_FILE,
    ERROR
  };

  /**
   * Set the name of the Connection. This should be unique, and will be used in
   * the GUI to refer to this Connection.
   */
  virtual void setName(const QString &name) { m_name = name; }

  /** Get the name of the Connection. */
  QString name() const { return m_name; }

signals:
  /** The client disconnected. */
  void disconnect();

  /** The client submitted a new job. */
  void jobSubmitted(const QString &queue, const QString &program,
                    const QString &fileName, const QString &input);

public slots:
  /**
   * Slot for receiving new data from the client.
   */
  void newDataReady();

  /**
   * Send a message to the client.
   */
  void sendMessage(const QString &messgae);

protected:
  QString m_name;
  QMap<QString, Program> m_programs;
  QList<Program *> m_jobs;

  QLocalSocket *m_socket;
  quint16 m_blockSize;

  State m_state;
};

} // End namespace

#endif // Connection_H
