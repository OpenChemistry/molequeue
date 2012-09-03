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

#include "authenticatecont.h"

#include <QtCore/QXmlStreamWriter>

namespace MoleQueue
{

AuthenticateCont::AuthenticateCont(const QString authSessionId,
                                   const QList<Prompt> prompts)
  : m_authSessionId(authSessionId), m_prompts(prompts)
{

}

QString AuthenticateCont::toXml() const
{
  QString xml;
  QXmlStreamWriter xmlWriter(&xml);

  xmlWriter.writeStartDocument();
  xmlWriter.writeStartElement("AuthenticateCont");

  xmlWriter.writeTextElement("auth__session__id", m_authSessionId);
  xmlWriter.writeStartElement("prompts");

  foreach(const Prompt &p, m_prompts) {
    xmlWriter.writeStartElement("Prompt");
    xmlWriter.writeTextElement("id", QString::number(p.id()));
    xmlWriter.writeTextElement("prompt", p.prompt());
    xmlWriter.writeTextElement("reply", p.userResponse());
    xmlWriter.writeEndElement();
  }

  xmlWriter.writeEndElement();
  xmlWriter.writeEndElement();

  return xml;

}

} /* namespace MoleQueue */
