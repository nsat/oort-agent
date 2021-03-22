from oort_sdk_client import SdkApi
from oort_sdk_client.models import SendFileRequest, RetrieveFileRequest

import sys

agent = SdkApi()

req = SendFileRequest(destination="ground", topic=sys.argv[1], filepath=sys.argv[2])
res = agent.send_file(req)

print(res.uuid)
