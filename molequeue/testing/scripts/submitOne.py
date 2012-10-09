import molequeue as mq
from getopt import getopt
import sys
import time

debug = False

socketname = ""
sentinal = False
moleQueueId = -1

def run_test():
  global sentinal, socketname, moleQueueId
  if debug:
    print "Connecting to socket: %s"%socketname
  client = mq.Client()
  client.connect_to_server(socketname)

  def notification_callback(msg):
    global sentinal, moleQueueId
    while (moleQueueId == None):
      pass
    try:
      if msg['method'] == 'jobStateChanged':
        if msg['params']['moleQueueId'] == moleQueueId:
          if msg['params']['newState'] == 'Finished':
            if debug:
              print "Great Job!!"
            sentinal = True
    except:
      print "Unexpected notification:", msg
    sys.stdout.flush()

  client.register_notification_callback(notification_callback)

  job = mq.JobRequest()
  job.queue   = "TestQueue"
  job.program = "TestProgram"
  job.description = "Test job from python client"
  job.popup_on_state_change = False
  moleQueueId = client.submit_job_request(job, 5)
  if moleQueueId == None:
    # Timeout
    client.disconnect()
    raise Exception("Connection timed out!")

  timeout = 20
  while sentinal == False and timeout > 0:
    timeout -= 1
    time.sleep(1)

  client.disconnect()
  if sentinal == True:
    return 0
  return 1

def main(argv):
  # -s for socketname
  opts, args = getopt(argv, "s:")
  for opt, arg in opts:
    if opt == "-s":
      global socketname
      socketname = arg

  try:
    test_result = run_test()
  except Exception as ex:
    print ex
    test_result = 1
  if debug:
    print "Exiting with code: %d"%test_result
  sys.stdout.flush()
  return test_result

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
