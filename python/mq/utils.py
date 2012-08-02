import json

class JsonRpc:
  @staticmethod
  def generate_request(molequeueid, method, parameters):
    request = {}
    request['jsonrpc'] = "2.0"
    request['id'] = molequeueid
    request['method'] = method
    request['params'] = parameters

    return json.dumps(request)

def underscore_to_camelcase(value):
    def camelcase():
        yield str.lower
        while True:
            yield str.capitalize

    c = camelcase()

    return "".join(c.next()(x) for x in value.split("_"))
