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

#ifndef MOLEQUEUE_ABSTRACTQUEUESETTINGSWIDGET_H
#define MOLEQUEUE_ABSTRACTQUEUESETTINGSWIDGET_H

#include <QtGui/QWidget>

namespace MoleQueue {

/// @brief Base interface for custom queue settings widgets.
class AbstractQueueSettingsWidget : public QWidget
{
  Q_OBJECT
public:
  explicit AbstractQueueSettingsWidget(QWidget *parentObject = 0);

  /// Has the GUI been modified from the current Queue state?
  bool isDirty() const { return m_isDirty; }

signals:

public slots:

  /// Write the information from the GUI to the Queue. Subclasses
  /// should call setDirty(false) at the end of their implementation.
  virtual void save() = 0;

  /// Update the Queue with the current configuration in the GUI. Subclasses
  /// should call setDirty(false) at the end of their implementation.
  virtual void reset() = 0;

protected slots:

  void setDirty(bool dirty = true) { m_isDirty = dirty; }

protected:
  bool m_isDirty;

};

} // namespace MoleQueue

#endif // MOLEQUEUE_ABSTRACTQUEUESETTINGSWIDGET_H
