import molequeue as mq
from getopt import getopt
import sys
import time

from threading import Lock

debug = False

numJobs = 1
socketname = ""
clientId = 0
moleQueueIdsDone = []
mqIdLock = Lock()

def run_test():
  global socketname, moleQueueIdsDone, clientId, mqIdLock, numJobs
  if debug:
    print "Client %s connecting to socket: %s"%(clientId, socketname)
  client = mq.Client()
  client.connect_to_server(socketname)

  def notification_callback(msg):
    global moleQueueIdsDone, mqIdLock
    try:
      if msg['method'] == 'jobStateChanged':
        if msg['params']['newState'] == 'Finished':
          moleQueueId = msg['params']['moleQueueId']
          with mqIdLock:
            moleQueueIdsDone.append(moleQueueId)
          if debug:
            print "Job %d finished! (Client %s)"%(moleQueueId, clientId)
    except Exception as ex:
      print "Unexpected notification:", msg, ex
    sys.stdout.flush()

  client.register_notification_callback(notification_callback)

  moleQueueIds = []
  for i in range(numJobs):
    job = mq.JobRequest()
    job.queue   = "TestQueue"
    job.program = "TestProgram"
    job.description = "Test job %d from python client %s"%(i+1, clientId)
    job.popup_on_state_change = False
    moleQueueId = client.submit_job_request(job, 30)
    moleQueueIds.append(moleQueueId)
    if moleQueueId == None:
      # Timeout
      client.disconnect()
      raise Exception("Connection timed out!")
    if debug:
      print "Submitted job %d (Client %s)"%(moleQueueId, clientId)
    sys.stdout.flush()

  timeout = 30
  mqIdLock.acquire()
  while len(moleQueueIds) != len(moleQueueIdsDone) and timeout > 0:
    if debug:
      print "Client %s waiting to finish (timeout=%d unmatchedIDs=%d)"%\
        (clientId, timeout, len(moleQueueIds) - len(moleQueueIdsDone))
    sys.stdout.flush()
    timeout -= 1
    mqIdLock.release()
    time.sleep(1)
    mqIdLock.acquire()
  mqIdLock.release()

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
      global numJobs
      numJobs = int(arg)

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
