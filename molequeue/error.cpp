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

#include "error.h"

namespace MoleQueue
{

Error::Error(const QString &message_, Error::ErrorType type_, Object *sender_,
             IdType moleQueueId_, const QVariant data_)
  : m_message(message_),
    m_type(type_),
    m_sender(sender_),
    m_moleQueueId(moleQueueId_),
    m_data(data_)
{
}

Error::Error(const Error &other)
  : m_message(other.m_message),
    m_type(other.m_type),
    m_sender(other.m_sender),
    m_moleQueueId(other.m_moleQueueId),
    m_data(other.m_data)
{
}

} // end namespace MoleQueue
