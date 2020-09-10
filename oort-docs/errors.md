# Errors

```python
from oort_sdk_client import SdkApi, ApiException, ErrorResponse
from oort_sdk_client.models import SendFileRequest, RetrieveFileRequest

import sys

agent = SdkApi()

req = SendFileRequest(
    destination="ground",
    topic=sys.argv[1],
    filepath=sys.argv[2])

try:
    res = agent.send_file(req)
    print(res.uuid)
except ApiException as e:
    print("Error sending file: {}".format(e.body))
```

Errors are returned as a [ErrorResponse](#errorresponse) value,
which contains an error code and a message.  
