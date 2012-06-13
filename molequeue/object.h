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

#ifndef OBJECT_H
#define OBJECT_H

#include <QtCore/QObject>

namespace MoleQueue
{
class Error;

class Object : public QObject
{
  Q_OBJECT
public:
  explicit Object(QObject *parent_ = NULL);
  virtual ~Object();

  Object * objectParent() const { return m_objectParent; }

signals:

  /**
   * Emitted when an error occurs.
   * @param err Error object containing information about the error.
   */
  void errorOccurred(const MoleQueue::Error &err);

public slots:

  /**
   * Called when a subclass Object emits the errorOccurred signal. The default
   * implementation simply re-emits the error to allow the parent to handle it.
   * @param err Error object containing information about the error.
   */
  virtual void handleError(const MoleQueue::Error &err);

protected:
  Object *m_objectParent;
};

} // end namespace MoleQueue

#endif // OBJECT_H
