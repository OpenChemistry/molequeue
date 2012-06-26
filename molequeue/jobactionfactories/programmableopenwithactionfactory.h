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

#ifndef PROGRAMMABLEOPENWITHACTIONFACTORY_H
#define PROGRAMMABLEOPENWITHACTIONFACTORY_H

#include "openwithactionfactory.h"

#include <QtCore/QList>
#include <QtCore/QRegExp>

namespace MoleQueue
{

class ProgrammableOpenWithActionFactory : public OpenWithActionFactory
{
  Q_OBJECT
public:
  ProgrammableOpenWithActionFactory();
  ProgrammableOpenWithActionFactory(const ProgrammableOpenWithActionFactory &);
  ProgrammableOpenWithActionFactory &operator=(const ProgrammableOpenWithActionFactory &);
  virtual ~ProgrammableOpenWithActionFactory();

  virtual void readSettings(QSettings &settings);
  virtual void writeSettings(QSettings &settings) const;

  void setExecutableName(const QString &name) { m_executableName = name; }

  void setRecognizedFilePatterns(const QList<QRegExp> &patterns);
  const QList<QRegExp> & recognizedFilePatterns() const;
  QList<QRegExp> & recognizedFilePatterns();

  virtual bool isValidForJob(const Job *job) const;

protected:
  QList<QRegExp> m_recognizedFilePatterns;
};

} // end namespace MoleQueue

#endif // PROGRAMMABLEOPENWITHACTIONFACTORY_H
