import unittest

import molequeue

class TestClient(unittest.TestCase):

  def test_submit_job_request(self):
    client = molequeue.Client()
    client.connect_to_server('MoleQueue')

    job_request = molequeue.JobRequest()
    job_request.queue = 'salix'
    job_request.program = 'sleep (testing)'

    client.submit_job_request(job_request)

if __name__ == '__main__':
    unittest.main()
