## Ground-side

```python
import time
import os.path

from oort_sdk_client import SdkApi
from oort_sdk_client.models import RetrieveFileRequest

from custom_application.ground import process_logs, upload_data

agent = SdkApi()

# these topic names will be provided by the Spire Constellation Ops team
oort_topic_primary = "custom-application"
oort_topic_logs = "custom-application-logs"
oort_topic_raw = "custom-application-raw"


# poll the OORT agent for new files
while True:

    available_primary = agent.query_available_files(oort_topic_primary)
    if not available_primary.files:
        print("No files available for retreival from {}"
            .format(oort_topic_primary))
    else:
        for item in available_primary.files:
            print("Retrieving {id} ({path})".format(item.id, item.path))
            save_path = '/tmp/{}'.format(os.path.basename(item.path))
            req = RetrieveFileRequest(
                id=item.id,
                save_path=save_path)
            agent.retrieve_file(req)

            # the application can now upload or process the data as
            # required
            upload_data(save_path, "processed")

    available_raw = agent.query_available_files(oort_topic_raw)
    if not available_raw.files:
        print("No files available for retreival from {}"
            .format(oort_topic_raw))
    else:
        for item in available_raw.files:
            print("Retrieving {id} ({path})".format(item.id, item.path))
            save_path = '/tmp/{}'.format(os.path.basename(item.path))
            req = RetrieveFileRequest(
                id=item.id,
                save_path=save_path)
            agent.retrieve_file(req)

            upload_data(save_path, "raw")

    available_logs = agent.query_available_files(oort_topic_logs)
    if not available_logs.files:
        print("No logs available for retreival")
    else:
        for item in available_logs.files:
            print("Retrieving {id} ({path})".format(item.id, item.path))
            save_path = '/tmp/{}'.format(os.path.basename(item.path))
            req = RetrieveFileRequest(
                id=item.id,
                save_path=save_path)
            agent.retrieve_file(req)

            process_logs(save_path)

    # sleep before checking for files again
    time.sleep(60)
```

After the OORT pipeline has transferred sent files to the ground, they are
retrieved for further processing.  The files will be delivered in the
original format they were sent in, so if they were sent compressed, they
will be received compressed.  Any transformations the OORT pipeline makes
are transparent, and the client application does not need to be concerned
with them.

The steps for retrieving the files are the same regardless of what
options they were sent with, so the receiving application must know
how to handle files differently based on topic, filename, or 
some method of content type detection.  In this example, the topic is 
used to differentiate the file types.
