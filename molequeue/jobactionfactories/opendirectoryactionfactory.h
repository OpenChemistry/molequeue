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

#ifndef OPENDIRECTORYACTIONFACTORY_H
#define OPENDIRECTORYACTIONFACTORY_H

#include "../jobactionfactory.h"

namespace MoleQueue
{

/// JobActionFactory subclass to open job output in a file browser.
class OpenDirectoryActionFactory : public JobActionFactory
{
  Q_OBJECT
public:
  OpenDirectoryActionFactory();
  ~OpenDirectoryActionFactory();

  bool isValidForJob(const Job &job) const;

  QList<QAction*> createActions();

  unsigned int usefulness() const { return 300; }

protected slots:
  void actionTriggered();
};

} // end namespace MoleQueue

#endif // OPENDIRECTORYACTIONFACTORY_H
