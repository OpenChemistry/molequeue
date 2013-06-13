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

#include <QtCore/QXmlStreamWriter>

#include "filestreamingdata.h"

namespace MoleQueue {
namespace Uit {

FileStreamingData::FileStreamingData()
  : m_hostID(-1)
{

}

QString FileStreamingData::toXml() const
{
  QString xml;
  QXmlStreamWriter xmlWriter(&xml);

  xmlWriter.writeStartDocument();
  xmlWriter.writeStartElement("FileStreamingData");

  xmlWriter.writeTextElement("token", m_token);
  xmlWriter.writeTextElement("filename", m_fileName);
  xmlWriter.writeTextElement("hostID", QString::number(m_hostID));
  xmlWriter.writeTextElement("username", m_userName);

  xmlWriter.writeEndElement();

  return xml;
}

} /* namespace Uit */
} /* namespace MoleQueue */
