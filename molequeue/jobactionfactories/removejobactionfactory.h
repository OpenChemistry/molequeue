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

namespace MoleQueue {

class RemoveJobActionFactory : public JobActionFactory
{
  Q_OBJECT
public:
  explicit RemoveJobActionFactory();
  virtual ~RemoveJobActionFactory();

  virtual bool isValidForJob(const Job *job) const;

  virtual QList<QAction*> createActions();

  virtual unsigned int usefulness() const { return 200; }

public slots:
  virtual void actionTriggered();
};

} // namespace MoleQueue

#endif // MOLEQUEUE_REMOVEJOBACTIONFACTORY_H
