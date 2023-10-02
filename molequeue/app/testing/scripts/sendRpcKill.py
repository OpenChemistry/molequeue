import molequeue as mq
from getopt import getopt
import sys
import time

debug = False

socketname = ""

def run_test():
  global socketname
  if debug:
    print("Connecting to socket: %s"%socketname)
  client = mq.Client()
  client.connect_to_server(socketname)
  res = client._send_rpc_kill_request(5)
  client.disconnect()
  if res == None:
    raise Exception("Connection timed out!")
  if res != True:
    return 1
  return 0

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
    print(ex)
    test_result = 1
  if debug:
    print("Exiting with code: %d"%test_result)
  sys.stdout.flush()
  return test_result

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
