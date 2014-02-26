/******************************************************************************

 This source file is part of the MoleQueue project.

 Copyright 2013 Kitware, Inc.

 This source code is released under the New BSD License, (the "License").

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 ******************************************************************************/

#ifndef MOLEQUEUE_ABOUTDIALOG_H
#define MOLEQUEUE_ABOUTDIALOG_H

#include <QtWidgets/QDialog>

namespace Ui {
class AboutDialog;
}

namespace MoleQueue
{

class AboutDialog : public QDialog
{
  Q_OBJECT
public:
  AboutDialog(QWidget* parent = 0);
  ~AboutDialog();

private:
  Ui::AboutDialog *m_ui;
};

} // End namespace

#endif
