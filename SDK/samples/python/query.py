import os
import sys
import time

from oort_sdk_client import SdkApi
from oort_sdk_client.models import SendFileRequest, RetrieveFileRequest

agent = SdkApi()

topic = sys.argv[1]

s_time = time.time()

available = agent.query_available_files(topic).files
if not available:
    print("No files available for retreival")
else:
    for f in available:
        print("Retrieving {} ({})".format(f.id, f.path))
        req = RetrieveFileRequest(
            id=f.id,
            save_path='/tmp/{}'.format(os.path.basename(f.path)))
        agent.retrieve_file(req)

e_time = time.time()
print("Took {:3f} seconds".format(e_time - s_time))
