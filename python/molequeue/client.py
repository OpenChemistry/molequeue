import zmq
from zmq.eventloop import ioloop
from zmq.eventloop.zmqstream import ZMQStream
from threading import Thread
from threading import Condition
from threading import Lock
from functools import partial
import inspect
import json
import time
import tempfile
import sys

from utils import underscore_to_camelcase
from utils import camelcase_to_underscore
from utils import JsonRpc
import threading

class JobState:
  # Unknown status
  UNKNOWN = -1,
  # Initial state of job, should never be entered.
  NONE = 0,
  # Job has been accepted by the server and is being prepared (Writing input files, etc).
  ACCEPTED = 1
  # Job is being queued locally, either waiting for local execution or remote submission.
  LOCAL_QUEUED = 2
  # Job has been submitted to a remote queuing system.
  SUBMITTED = 3
  # Job is pending execution on a remote queuing system.
  REMOTE_QUEUED = 4
  # Job is running locally.
  RUNNING_LOCAL = 5
  # Job is running remotely.
  RUNNING_REMOTE = 6
  # Job has completed.
  FINISHED = 7
  # Job has been terminated at a user request.
  KILLED = 8
  # Job has been terminated due to an error.
  ERROR = 9

class FilePath:
  def __init__(self):
    self.path = None

class FileContents:
  def __init__(self):
    self.filename = None
    self.contents = None

class JobRequest:
  def __init__(self):
    self.queue = None
    self.program = None
    self.description = ''
    self.input_file = None
    self.output_directory = None
    self.local_working_directory = None
    self.clean_remote_files = False
    self.retrieve_output = True
    self.clean_local_working_directory = False
    self.hide_from_gui = False
    self.popup_on_state_change = True
    self.number_of_cores = 1
    self.max_wall_time = -1

  def job_state(self):
    return self._job_state

  def molequeue_id(self):
    return self._mole_queue_id

  def queue_id(self):
    return self._queue_id

class Queue:
  def __init__(self):
    self.name = None;
    self.programs = [];

class EventLoop(Thread):
  def __init__(self, io_loop):
    Thread.__init__(self)
    self.io_loop = io_loop

  def run(self):
    self.io_loop.start()

  def stop(self):
    self.io_loop.stop()

class MoleQueueException(Exception):
  """The base class of all MoleQueue exceptions """
  pass

class JobRequestException(MoleQueueException):
  def __init__(self, packet_id, code, message):
    self.packet_id = packet_id
    self.code = code
    self.message = message

class JobRequestInformationException(MoleQueueException):
  def __init__(self, packet_id, data, code, message):
    self.packet_id = packet_id
    self.data = data
    self.code = code
    self.message = message

class Client:

  def __init__(self):
    self._current_packet_id = 0
    self._request_response_map = {}
    self._new_response_condition = Condition()
    self._packet_id_lock = Lock()
    self._notification_callbacks = []

  def connect_to_server(self, server):
    self.context = zmq.Context()
    self.socket = self.context.socket(zmq.DEALER)

    tmpdir = tempfile.gettempdir()
    connection_string  = 'ipc://%s/%s_%s' %  (tmpdir, 'zmq', server)
    self.socket.connect(connection_string)

    io_loop = ioloop.IOLoop(ioloop.ZMQPoller())

    self.stream = ZMQStream(self.socket, io_loop)

    # create partial function that has self as first argument
    callback = partial(_on_recv, self)
    self.stream.on_recv(callback)
    self.event_loop = EventLoop(io_loop)
    self.event_loop.start()

  def disconnect(self):
    self.stream.flush()
    self.event_loop.stop()
    self.socket.close()

  def register_notification_callback(self, callback):
    # check a valid function has been past
    assert callable(callback)
    self._notification_callbacks.append(callback)

  def request_queue_list_update(self, timeout=None):
    packet_id = self._next_packet_id()

    jsonrpc = JsonRpc.generate_request(packet_id,
                                       'listQueues',
                                       None)

    self._send_request(packet_id, jsonrpc)
    response = self._wait_for_response(packet_id, timeout)

    # Timeout
    if response == None:
      return None

    queues = JsonRpc.json_to_queues(response)

    return queues

  def submit_job_request(self, request, timeout=None):
    params = JsonRpc.object_to_json_params(request)
    packet_id = self._next_packet_id()

    jsonrpc = JsonRpc.generate_request(packet_id,
                                       'submitJob',
                                       params)

    self._send_request(packet_id, jsonrpc)
    response = self._wait_for_response(packet_id, timeout)

    # Timeout
    if response == None:
      return None

    # if we an error occurred then throw an exception
    if 'error' in response:
      exception = JobRequestException(response['id'],
                                      response['error']['code'],
                                      response['error']['message'])
      raise exception

    # otherwise return the molequeue id
    return response['result']['moleQueueId']

  def cancel_job(self):
    # TODO
    pass

  def lookup_job(self, molequeue_id, timeout=None):

    params = {'moleQueueId': molequeue_id}

    packet_id = self._next_packet_id()
    jsonrpc = JsonRpc.generate_request(packet_id,
                                      'lookupJob',
                                      params)

    self._send_request(packet_id, jsonrpc)
    response = self._wait_for_response(packet_id, timeout)

    # Timeout
    if response == None:
      return None

    # if we an error occurred then throw an exception
    if 'error' in response:
      exception = JobRequestInformationException(response['id'],
                                                 response['error']['data'],
                                                 response['error']['code'],
                                                 response['error']['message'])
      raise exception

    jobrequest = JsonRpc.json_to_jobrequest(response)

    return jobrequest


  def _on_response(self, packet_id, msg):
    if packet_id in self._request_response_map:
      self._new_response_condition.acquire()
      self._request_response_map[packet_id] = msg
      # notify any threads waiting that their response may have arrived
      self._new_response_condition.notify_all()
      self._new_response_condition.release()

  # TODO Convert raw JSON into a Python class
  def _on_notification(self, msg):
    for callback in self._notification_callbacks:
      callback(msg)

  # Testing only method. Kill the server application if allowed.
  def _send_rpc_kill_request(self, timeout=None):
    params = {}
    packet_id = self._next_packet_id()
    jsonrpc = JsonRpc.generate_request(packet_id, 'rpcKill', params)
    self._send_request(packet_id, jsonrpc)
    response = self._wait_for_response(packet_id, timeout)

    # Timeout
    if response == None:
      return None

    if 'result' in response and 'success' in response['result'] and response['result']['success'] == True:
      return True
    return False

  def _next_packet_id(self):
    with self._packet_id_lock:
      self._current_packet_id += 1
      next = self._current_packet_id
    return next

  def _send_request(self, packet_id, jsonrpc):
    # add to request map so we know we are waiting on  response for this packet
    # id
    self._request_response_map[packet_id] = None
    self.stream.send(str(jsonrpc))
    self.stream.flush()

  def _wait_for_response(self, packet_id, timeout):
    try:
      start = time.time()
      # wait for the response to come in
      self._new_response_condition.acquire()
      while self._request_response_map[packet_id] == None:
        # need to set a wait time otherwise the wait can't be interrupted
        # See http://bugs.python.org/issue8844
        wait_time = sys.maxint
        if timeout != None:
          wait_time = timeout - (time.time() - start)
          if wait_time <= 0:
            break;
        self._new_response_condition.wait(wait_time)

      response = self._request_response_map.pop(packet_id)

      self._new_response_condition.release()

      return response
    except KeyboardInterrupt:
      self.event_loop.stop()
      raise

def _on_recv(client, msg):
  jsonrpc = json.loads(msg[0])

  # reply to a request
  if 'id' in jsonrpc:
    packet_id = jsonrpc['id']
    client._on_response(packet_id, jsonrpc)
  # this is a notification
  else:
    client._on_notification(jsonrpc)
