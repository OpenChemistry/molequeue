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

#ifndef MOLEQUEUE_VIEWJOBLOGACTIONFACTORY_H
#define MOLEQUEUE_VIEWJOBLOGACTIONFACTORY_H

#include "jobactionfactory.h"

#include "molequeueglobal.h"

namespace MoleQueue {
class LogWindow;

/// @brief JobActionFactory for opening log windows filtered to specified jobs.
class ViewJobLogActionFactory : public MoleQueue::JobActionFactory
{
  Q_OBJECT
public:
  ViewJobLogActionFactory();
  ~ViewJobLogActionFactory();

  QString name() const { return tr("View log"); }

  bool isValidForJob(const Job &job) const;

  QList<QAction*> createActions();

  void setLogWindowParent(QWidget *widgy);

  unsigned int usefulness() const { return 50; }

public slots:
  virtual void actionTriggered();
  void removeSenderFromMap();

private:
  QWidget *m_logWindowParent;
  QMap<IdType, LogWindow*> m_windowMap;
};

} // namespace MoleQueue

#endif // MOLEQUEUE_VIEWJOBLOGACTIONFACTORY_H
