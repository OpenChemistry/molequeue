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

#include "queues/uit/dirlistinginfo.h"
#include "logger.h"
#include "fileinfo.h"
#include "messagehandler.h"

#include <QtXmlPatterns/QAbstractXmlReceiver>
#include <QtXmlPatterns/QXmlNamePool>
#include <QtXmlPatterns/QXmlQuery>
#include <QtCore/QList>

namespace {

class FileInfoXmlReceiver : public QAbstractXmlReceiver
{

public:
  FileInfoXmlReceiver(QXmlNamePool pool);

  void  atomicValue ( const QVariant & value ) { Q_UNUSED(value); }
  void  attribute ( const QXmlName & name, const QStringRef & value ) {
    Q_UNUSED(name);
    Q_UNUSED(value);
  }
  void  characters ( const QStringRef & value );
  void  comment ( const QString & value ) { Q_UNUSED(value); }
  void  endDocument () {};
  void  endElement ();
  void  endOfSequence () {};
  void  namespaceBinding ( const QXmlName & name ) { Q_UNUSED(name); }
  void  processingInstruction ( const QXmlName & target,
                               const QString & value) {
    Q_UNUSED(target);
    Q_UNUSED(value);
  }
  void  startDocument () {};
  void  startElement ( const QXmlName & name );
  void  startOfSequence () {};

  QList<MoleQueue::Uit::FileInfo> fileInfos() const {
    return m_fileInfos;
  };

private:
  QXmlNamePool m_pool;
  MoleQueue::Uit::FileInfo m_currentFileInfo;
  QString m_currentName;
  QString m_currentValue;
  QList<MoleQueue::Uit::FileInfo> m_fileInfos;
  int tagDepth;

};

FileInfoXmlReceiver::FileInfoXmlReceiver(QXmlNamePool pool)
  : m_pool(pool), tagDepth(0)
{

}

void  FileInfoXmlReceiver::characters ( const QStringRef & value )
{
  if (!m_currentName.isEmpty()) {
    m_currentValue.append(value);
  }
}

void  FileInfoXmlReceiver::endElement ()
{
  if (tagDepth-- == 2) {
    m_fileInfos.append(m_currentFileInfo);
    m_currentFileInfo = MoleQueue::Uit::FileInfo();
  }
  else if (m_currentName == "size") {
    bool ok;
    m_currentFileInfo.setSize(m_currentValue.toLongLong(&ok));

    if (!ok) {
      MoleQueue::Logger::logError(
          QObject::tr("Unable to convert value to qint64: %1")
                       .arg(m_currentValue));
    }
  }
  else if (m_currentName == "name") {
    m_currentFileInfo.setName(m_currentValue);
  }
  else if (m_currentName == "perms") {
    m_currentFileInfo.setPerms(m_currentValue);
  }
  else if (m_currentName == "date") {
    m_currentFileInfo.setDate(m_currentValue);
  }
  else if (m_currentName == "user") {
    m_currentFileInfo.setUser(m_currentValue);
  }
  else if (m_currentName == "group") {
    m_currentFileInfo.setGroup(m_currentValue);
  }

  m_currentValue.clear();
  m_currentName.clear();
}

void  FileInfoXmlReceiver::startElement ( const QXmlName & name )
{
  tagDepth++;
  m_currentName =  name.localName(m_pool);
  m_currentValue.clear();
}

} /* namespace anonymous */

namespace MoleQueue {
namespace Uit {

DirListingInfo::DirListingInfo()
  : m_valid(false)
{
  // TODO Auto-generated constructor stub

}

DirListingInfo::DirListingInfo(const DirListingInfo &other)
  : m_valid(other.isValid()), m_currentDirectory(other.currentDirectory()),
    m_directories(other.directories()), m_files(other.files())
{

}

DirListingInfo DirListingInfo::fromXml(const QString &xml)
{
  DirListingInfo info;
  info.setContent(xml);

  return info;
}

void DirListingInfo::setContent(const QString &content)
{
  m_xml = content;
  m_valid = true;

  MessageHandler handler;
  QXmlQuery query;
  query.setMessageHandler(&handler);
  query.setFocus(m_xml);

  QString dir;
  query.setQuery("/DirListingInfo/currentDirectory/string()");
  m_valid = query.evaluateTo(&dir);

  if (!m_valid)
    return;

  m_currentDirectory = dir.trimmed();

  // Get the directories;
  FileInfoXmlReceiver dirReceiver(query.namePool());
  query.setQuery("/DirListingInfo/directories");
  m_valid = query.evaluateTo(&dirReceiver);

  if (!m_valid)
    return;

  m_directories = dirReceiver.fileInfos();

  // Get the files
  FileInfoXmlReceiver fileReceiver(query.namePool());
  query.setQuery("/DirListingInfo/files");
  m_valid = query.evaluateTo(&fileReceiver);

  if (!m_valid)
    return;

  m_files = fileReceiver.fileInfos();
}

} /* namespace Uit */
} /* namespace MoleQueue */
