import unittest
from functools import partial
import time

import molequeue

class TestClient(unittest.TestCase):

  def test_submit_job(self):
    client = molequeue.Client()
    client.connect_to_server('MoleQueue')

    job = molequeue.Job()
    job.queue = 'salix'
    job.program = 'sleep (testing)'

    file_path = molequeue.FilePath()
    file_path.path = "/tmp/test"

    job.input_file = file_path

    molequeue_id = client.submit_job(job)

    print("MoleQueue ID: ", molequeue_id)

    self.assertTrue(isinstance(molequeue_id, int))

    client.disconnect()

  def test_notification_callback(self):
    client = molequeue.Client()
    client.connect_to_server('MoleQueue')

    self.callback_count = 0

    def callback_counter(testcase, msg):
      testcase.callback_count +=1

    callback = partial(callback_counter, self)

    client.register_notification_callback(callback)
    client.register_notification_callback(callback)

    job = molequeue.Job()
    job.queue = 'salix'
    job.program = 'sleep (testing)'

    molequeue_id = client.submit_job(job)

    # wait for notification
    time.sleep(1)

    self.assertIs(self.callback_count, 2)

    client.disconnect()

  def test_wait_for_response_timeout(self):
     client = molequeue.Client()
     # Fake up the request
     client._request_response_map[1] = None
     start = time.time()
     response = client._wait_for_response(1, 3)
     end = time.time()

     self.assertEqual(response, None)
     self.assertEqual(int(end - start), 3)

  def test_lookup_job(self):
    client = molequeue.Client()
    client.connect_to_server('MoleQueue')

    expected_job = molequeue.Job()
    expected_job.queue = 'salix'
    expected_job.program = 'sleep (testing)'
    expected_job.description = 'This is a test job'
    expected_job.hide_from_gui = True
    expected_job.popup_on_state_change = False

    file_contents = molequeue.FileContents()
    file_contents.filename = 'test.in'
    file_contents.contents = 'Hello'
    expected_job.input_file = file_contents

    molequeue_id = client.submit_job(expected_job)

    job = client.lookup_job(molequeue_id)

    self.assertEqual(molequeue_id, job.molequeue_id())
    self.assertEqual(job.job_state(), molequeue.JobState.ACCEPTED)
    self.assertEqual(job.queue_id(), None)
    self.assertEqual(job.queue, expected_job.queue)
    self.assertEqual(job.program, expected_job.program)
    self.assertEqual(job.description, expected_job.description)
    self.assertEqual(job.hide_from_gui,
                     expected_job.hide_from_gui)
    self.assertEqual(job.popup_on_state_change,
                     expected_job.popup_on_state_change)

    client.disconnect()

  def test_request_queue_list_update(self):
    client = molequeue.Client()
    client.connect_to_server('MoleQueue')

    queues = client.request_queue_list_update()

    for q in queues:
      print(q.name, ", ", q.programs)

    client.disconnect()


if __name__ == '__main__':
    unittest.main()
