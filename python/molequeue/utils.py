import json
import re
import itertools
import molequeue

class JsonRpc:
  INTERNAL_FIELDS = ['moleQueueId', 'queueId', 'jobState']

  @staticmethod
  def generate_request(packet_id, method, parameters):
    request = {}
    request['jsonrpc'] = "2.0"
    request['id'] = packet_id
    request['method'] = method
    request['params'] = parameters

    return json.dumps(request)

  @staticmethod
  def json_to_jobrequest(json):
    jobrequest = molequeue.JobRequest()
    # convert response into JobRequest object
    for key, value in json['result'].iteritems():
      field = camelcase_to_underscore(key)
      if key in JsonRpc.INTERNAL_FIELDS:
        field = '_' + field
      jobrequest.__dict__[field] = value

    return jobrequest

  @staticmethod
  def jobrequest_to_json_params(jobrequest):
    params = {}
    for key, value in jobrequest.__dict__.iteritems():
      field = underscore_to_camelcase(key)
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
