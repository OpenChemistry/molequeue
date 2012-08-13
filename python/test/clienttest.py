import unittest
from functools import partial
import time

import molequeue

class TestClient(unittest.TestCase):

  def test_submit_job_request(self):
    client = molequeue.Client()
    client.connect_to_server('MoleQueue')

    job_request = molequeue.JobRequest()
    job_request.queue = 'salix'
    job_request.program = 'sleep (testing)'

    molequeue_id = client.submit_job_request(job_request)

    print "MoleQueue ID: ", molequeue_id

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

    job_request = molequeue.JobRequest()
    job_request.queue = 'salix'
    job_request.program = 'sleep (testing)'

    molequeue_id = client.submit_job_request(job_request)

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

    expected_job_request = molequeue.JobRequest()
    expected_job_request.queue = 'salix'
    expected_job_request.program = 'sleep (testing)'
    expected_job_request.description = 'This is a test job'
    expected_job_request.hide_from_gui = True
    expected_job_request.popup_on_state_change = False

    molequeue_id = client.submit_job_request(expected_job_request)

    jobrequest = client.lookup_job(molequeue_id)

    self.assertEqual(molequeue_id, jobrequest.molequeue_id())
    self.assertEqual(jobrequest.job_state(), molequeue.JobState.ACCEPTED)
    self.assertTrue(isinstance(jobrequest.queue_id(), int))
    self.assertEqual(jobrequest.queue, expected_job_request.queue)
    self.assertEqual(jobrequest.program, expected_job_request.program)
    self.assertEqual(jobrequest.description, expected_job_request.description)
    self.assertEqual(jobrequest.hide_from_gui,
                     expected_job_request.hide_from_gui)
    self.assertEqual(jobrequest.popup_on_state_change,
                     expected_job_request.popup_on_state_change)

    client.disconnect()

if __name__ == '__main__':
    unittest.main()
