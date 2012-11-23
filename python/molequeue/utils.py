import json
import re
import itertools
import types
import molequeue

class JsonRpc:
  INTERNAL_FIELDS = ['moleQueueId', 'queueId', 'jobState']

  @staticmethod
  def generate_request(packet_id, method, parameters):
    request = {}
    request['jsonrpc'] = "2.0"
    request['id'] = packet_id
    request['method'] = method
    if parameters != None:
      request['params'] = parameters

    return json.dumps(request)

  @staticmethod
  def json_to_job(json):
    job = molequeue.Job()
    # convert response into Job object
    for key, value in json['result'].iteritems():
      field = camelcase_to_underscore(key)
      if key in JsonRpc.INTERNAL_FIELDS:
        field = '_' + field
      job.__dict__[field] = value

    return job

  @staticmethod
  def json_to_queues(json):
    queues = []

    for name, programs in json['result'].iteritems():
      queue = molequeue.Queue();
      queue.name = name
      queue.programs = programs
      queues.append(queue)

    return queues;

  @staticmethod
  def object_to_json_params(job):
    params = {}

    for key, value in job.__dict__.iteritems():
      field = underscore_to_camelcase(key)

      if type(value) == types.InstanceType:
        value = JsonRpc.object_to_json_params(value)

      params[field] = value

    return params

def underscore_to_camelcase(value):
  def camelcase():
    yield str.lower
    while True:
      yield str.capitalize

  c = camelcase()

  return ''.join(c.next()(x) for x in value.split('_'))

def camelcase_to_underscore(value):
  operation = itertools.cycle((lambda x : x.lower(), lambda x : '_' + x.lower()))
  return ''.join(operation.next()(x) for x in re.split('([A-Z])', value))
