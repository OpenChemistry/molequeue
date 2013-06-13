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

#ifndef SSLSETUP_H_
#define SSLSETUP_H_

namespace MoleQueue {
namespace Uit {

/**
 * @brief class used to initalize SSL certificates for QSslSocket.
 */
class SslSetup
{
private:
  SslSetup();

  static bool sslCertsLoaded;

public:
  static void init();
};

} /* namespace Uit */
} /* namespace MoleQueue */
#endif /* SSLSETUP_H_ */
