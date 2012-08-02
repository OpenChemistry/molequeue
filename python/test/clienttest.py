import mq

client = mq.Client()
client.connect_to_server('MoleQueue')

job_request = mq.JobRequest()
job_request.queue = 'salix'
job_request.program = 'sleep (testing)'

client.submit_job_request(job_request)