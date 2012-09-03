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

#include "kerberoscredentials.h"

#include <QtCore/QXmlStreamWriter>

namespace MoleQueue
{

KerberosCredentials::KerberosCredentials(const QString &principle,
                                         const QString &password)
  : m_principle(principle), m_password(password)
{

}

QString KerberosCredentials::toXml() const
{
  QString xml;
  QXmlStreamWriter xmlWriter(&xml);

  xmlWriter.writeStartDocument();
  xmlWriter.writeStartElement("KerberosCredentials");
  xmlWriter.writeTextElement("principal", m_principle);
  xmlWriter.writeTextElement("password", m_password);
  xmlWriter.writeEndElement();

  return xml;
}

} /* namespace MoleQueue */
