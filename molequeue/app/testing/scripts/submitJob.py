import molequeue as mq
from getopt import getopt
import sys
import time

from threading import Lock

debug = False

num_jobs = 1
socketname = ""
clientId = 0
molequeue_ids_done = []
mq_id_lock = Lock()

def run_test():
  global socketname, molequeue_ids_done, clientId, mq_id_lock, num_jobs
  if debug:
    print "Client %s connecting to socket: %s"%(clientId, socketname)
  client = mq.Client()
  client.connect_to_server(socketname)

  def notification_callback(msg):
    global molequeue_ids_done, mq_id_lock
    try:
      if msg['method'] == 'jobStateChanged':
        if msg['params']['newState'] == 'Finished':
          moleQueueId = msg['params']['moleQueueId']
          with mq_id_lock:
            molequeue_ids_done.append(moleQueueId)
          if debug:
            print "Job %d finished! (Client %s)"%(moleQueueId, clientId)
    except Exception as ex:
      print "Unexpected notification:", msg, ex
    sys.stdout.flush()

  client.register_notification_callback(notification_callback)

  molequeue_ids = []
  for i in range(num_jobs):
    job = mq.Job()
    job.queue   = "TestQueue"
    job.program = "TestProgram"
    job.description = "Test job %d from python client %s"%(i+1, clientId)
    job.popup_on_state_change = False
    molequeue_id = client.submit_job(job, 30)
    molequeue_ids.append(molequeue_id)
    if molequeue_id == None:
      # Timeout
      client.disconnect()
      raise Exception("Connection timed out!")
    if debug:
      print "Submitted job %d (Client %s)"%(molequeue_id, clientId)
    sys.stdout.flush()

  timeout = 30
  mq_id_lock.acquire()
  while len(molequeue_ids) != len(molequeue_ids_done) and timeout > 0:
    if debug:
      print "Client %s waiting to finish (timeout=%d unmatchedIDs=%d)"%\
        (clientId, timeout, len(molequeue_ids) - len(molequeue_ids_done))
    sys.stdout.flush()
    timeout -= 1
    mq_id_lock.release()
    time.sleep(1)
    mq_id_lock.acquire()
  mq_id_lock.release()

  client.disconnect()
  if timeout > 0:
    return 0
  return 1

def main(argv):
  # -s for socketname
  # -c for clientId
  # -n for the number of jobs
  opts, args = getopt(argv, "s:c:n:")
  for opt, arg in opts:
    if opt == "-s":
      global socketname
      socketname = arg
    if opt == "-c":
      global clientId
      clientId = arg
    if opt == "-n":
      global num_jobs
      num_jobs = int(arg)

  try:
    test_result = run_test()
  except Exception as ex:
    print ex
    test_result = 1
  if debug:
    print "Exiting with code: %d (client %s)"%(test_result, clientId)
  sys.stdout.flush()
  return test_result

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
