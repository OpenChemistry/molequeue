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

#include "object.h"

#include "error.h"

#include <QtCore/QMetaType>

namespace MoleQueue
{

Object::Object(QObject *parent_) :
  QObject(parent_),
  m_objectParent(qobject_cast<Object*>(parent_))
{
  qRegisterMetaType<Error>("MoleQueue::Error");

  if (m_objectParent) {
    connect(this, SIGNAL(errorOccurred(MoleQueue::Error)),
            m_objectParent, SLOT(handleError(MoleQueue::Error)));
  }
}

Object::~Object()
{
}

void Object::handleError(const Error &err)
{
  emit errorOccurred(err);
}

} // end namespace MoleQueue
