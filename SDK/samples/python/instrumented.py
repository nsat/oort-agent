def my_mem_size():
    with open("/proc/self/status") as stat:
        for line in stat:
            if "VmRSS" in line:
                start = line.rfind(" ", 0, -4)
                end = line.find(" ", start+1)
                return int(line[start:end])
        return -1

print("Starting memory: {}".format(my_mem_size()))

import urllib3
print("Memory after urllib3: {}".format(my_mem_size()))


import os
import time

from oort_sdk_client import SdkApi
from oort_sdk_client.models import SendFileRequest, RetrieveFileRequest

print("Memory size after loading client: {}".format(my_mem_size()))

agent = SdkApi()

print("Memory size after instantiating client: {}".format(my_mem_size()))

for _ in range(10):
    s_time = time.time()

    available = agent.query_available_files("dropbox01").files
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
    time.sleep(1)

print("Memory size after making api calls: {}".format(my_mem_size()))
