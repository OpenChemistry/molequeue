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

#include "compositeiodevice.h"
#include "logger.h"

namespace MoleQueue {
namespace Uit {

CompositeIODevice::CompositeIODevice(QObject *parentObject)
  : QIODevice(parentObject), m_deviceIndex(0)
{

}

bool CompositeIODevice::addDevice(QIODevice *device)
{
  bool added = false;
  // The device should be readable.
  if (device->isReadable()) {
    m_devices.append(device);
    added = true;
  }

  return added;
}

qint64 CompositeIODevice::readData(char* data, qint64 maxSize)
{
  // We have no more devices to read so we are done.
  if (m_deviceIndex >= m_devices.size())
    return -1;

  QIODevice *device = m_devices[m_deviceIndex];
  qint64 bytesRead = device->read(data, maxSize);
  while (bytesRead < maxSize) {
    // If the current device is done move on the next in the list.
    if (device->atEnd()) {
      m_deviceIndex++;
      if (m_deviceIndex < m_devices.size())
        device = m_devices[m_deviceIndex];
      else
        // No more devices to read
        break;
    }

    qint64 leftToRead = maxSize - bytesRead;
    bytesRead += device->read(data + bytesRead, leftToRead);
  }

  return bytesRead;
}

qint64 CompositeIODevice::writeData(const char * data, qint64 maxSize)
{
  Q_UNUSED(data);
  Q_UNUSED(maxSize);

  Logger::logError("writeData not supported");

  return -1;
}

qint64 CompositeIODevice::size () const
{
  qint64 totalSize = 0;

  foreach(const QIODevice *device, m_devices)
    totalSize += device->size();

  return totalSize;
}

} /* namespace Uit */
} /* namespace MoleQueue */
