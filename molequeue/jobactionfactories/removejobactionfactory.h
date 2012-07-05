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

#ifndef MOLEQUEUE_REMOVEJOBACTIONFACTORY_H
#define MOLEQUEUE_REMOVEJOBACTIONFACTORY_H

#include "../jobactionfactory.h"

namespace MoleQueue
{

/**
 * @class RemoveJobActionFactory removejobactionfactory.h
 * <molequeue/jobactionfactories/removejobactionfactory.h>
 * @brief JobActionFactory subclass which removes jobs from the Server
 * JobManager.
 * @author David C. Lonie
 */
class RemoveJobActionFactory : public JobActionFactory
{
  Q_OBJECT
public:
  RemoveJobActionFactory();
  ~RemoveJobActionFactory();

  bool isValidForJob(const Job &job) const;

  QList<QAction*> createActions();

  unsigned int usefulness() const { return 200; }

public slots:
  virtual void actionTriggered();
};

} // namespace MoleQueue

#endif // MOLEQUEUE_REMOVEJOBACTIONFACTORY_H
