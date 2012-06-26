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

class OpenWithActionFactory : public JobActionFactory
{
  Q_OBJECT
public:
  OpenWithActionFactory();
  OpenWithActionFactory(const OpenWithActionFactory &other);
  virtual ~OpenWithActionFactory();
  OpenWithActionFactory & operator=(const OpenWithActionFactory &other);

  virtual void readSettings(QSettings &settings);
  virtual void writeSettings(QSettings &settings) const;

  QString executableFilePath() const { return m_executableFilePath; }
  QString executableName() const { return m_executableName; }

  virtual void clearJobs();

  virtual bool isValidForJob(const Job *job) const;

  virtual bool hasValidActions() const;

  virtual QList<QAction*> createActions();

  virtual unsigned int usefulness() const;

protected slots:
  virtual void actionTriggered();

protected:
  /// Return true and set m_executablePath if executable @a exec found in $PATH.
  virtual bool searchPathForExecutable(const QString &exec);

  QString m_executableFilePath;
  QString m_executableName;
};

} // end namespace MoleQueue

#endif // OPENWITHACTIONFACTORY_H
