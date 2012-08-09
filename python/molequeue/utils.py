import json
import re
import itertools

class JsonRpc:
  @staticmethod
  def generate_request(packet_id, method, parameters):
    request = {}
    request['jsonrpc'] = "2.0"
    request['id'] = packet_id
    request['method'] = method
    request['params'] = parameters

    return json.dumps(request)

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
