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

#ifndef COMPOSITEIODEVICE_H_
#define COMPOSITEIODEVICE_H_

#include <QtCore/QIODevice>
#include <QtCore/QQueue>

namespace MoleQueue {
namespace Uit {

/**
 * @class CompositeIODevice compositeiodevice.h
 * <molequeue/queue/uit/compositeiodevice.h>
 * @brief The CompositeIODevice class is facade that allows several QIODevices
 * into a single QIODevice.
 *
 */
class CompositeIODevice: public QIODevice
{
  Q_OBJECT
public:
  CompositeIODevice(QObject *parentObject = 0);

  /**
   * Add a QIODevice to the device. The QIODevice being added must be open in
   * read mode.
   *
   * @param device The QIODevice to add.
   */
  bool addDevice(QIODevice *device);
  /**
   * @return The combine size of all the QIODevices this composite represents.
   */
  qint64 size () const;

protected:
  /**
   * Override superclass with composite read.
   */
  qint64 readData(char* data, qint64 maxSize);
  /**
   * Override superclass, write is not supported.
   */
  qint64  writeData ( const char * data, qint64 maxSize );

private:
  /// The list of QIODevices in the composite.
  QList<QIODevice *> m_devices;
  /// The index of the QIODevice currently being read.
  int m_deviceIndex;
};

} /* namespace Uit */
} /* namespace MoleQueue */
#endif /* COMPOSITEIODEVICE_H_ */
