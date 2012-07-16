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

#ifndef CONNECTIONTEST_H_
#define CONNECTIONTEST_H_

#include "queue.h"
#include "queuemanager.h"
#include "client.h"
#include "server.h"

#include <QtTest>


class QueueDummy : public MoleQueue::Queue
{
  Q_OBJECT
public:
  QueueDummy(MoleQueue::QueueManager *parentManager)
    : MoleQueue::Queue ("Dummy", parentManager)
  {
  }
public slots:
  bool submitJob(const MoleQueue::Job);
};

class ConnectionTest : public QObject
{
  Q_OBJECT
protected:
  virtual MoleQueue::Client *createClient() = 0;
private:
  QString m_connectionName;
  MoleQueue::Server *m_server;
  MoleQueue::Client *m_client;

private slots:

  /// Called before the first test function is executed.
  void initTestCase();
  /// Called after the last test function is executed.
  void cleanupTestCase();
  /// Called before each test function is executed.
  void init();
  /// Called after every test function.
  void cleanup();

  void testRequestQueueList();
  void testSuccessfulJobSubmission();
  void testFailedSubmission();
  void testSuccessfulJobCancellation();
  void testJobStateChangeNotification();

};

#endif /* CONNECTIONTEST_H_ */
