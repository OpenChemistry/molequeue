import zmq
from utils import underscore_to_camelcase
from utils import JsonRpc

class JobRequest:
  def __init__(self):
    self.queue = None
    self.program = None
    self.description = ''
    self.input_as_path = None
    self.input_as_string = None
    self.output_directory = None
    self.local_working_directory = None
    self.clean_remote_files = False
    self.retrieve_output = True
    self.clean_local_working_directory = False
    self.hide_from_gui = False
    self.pop_up_state_change = True

  def job_state(self):
    # TODO
    pass

  def molequeue_id(self):
    # TODO
    pass

  def queue_id(self):
    # TODO
    pass

class Client:

  def connect_to_server(self, server):
    self.context = zmq.Context()
    self.socket = self.context.socket(zmq.DEALER)
    self.socket.connect('ipc://%s' % server)

  def request_queue_list_update(self):
    pass

  def submit_job_request(self, request):
    params = {}
    for key, value in request.__dict__.iteritems():
      params[underscore_to_camelcase(key)] = value

    jsonrpc = JsonRpc.generate_request(1,
                                      'submitJob',
                                      params)
    self.socket.send(str(jsonrpc))

  def cancel_job(self):
    # TODO
    pass

  def lookup_job(self):
    # TODO
    pass
