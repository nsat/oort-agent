**Version:** 1.0

# Example
## Satellite-side

```python
from oort_sdk_client import SdkApi
from oort_sdk_client.models import SendFileRequest, SendOptions, TTLParams

from custom_application import observe, process, save_to_file, create_logs

# these topic names will be provided by the Spire Constellation Ops team
oort_topic_primary = "custom-application"
oort_topic_logs = "custom-application-logs"
oort_topic_raw = "custom-application-raw"

agent = SdkApi()

while True:
    raw_observation = observe()

    # on-board processing may be done to extract the most important data
    processed_observation = process(raw_observation)

    raw_filename = save_to_file(raw_observation)
    processed_filename = save_to_file(processed_observation)

    # send the important processed data
    req = SendFileRequest(
        destination="ground",
        topic=oort_topic_primary,
        filepath=processed_filename)
    resp = agent.send_file(req)

    # logfiles may be very useful, but not as critical as the important 
    # data observations.  Send those as "bulk" data

    # The hypothetical "create_logs" method would write any log files
    # in progress, and return a list of their filenames.

    log_files = create_logs()
    ttl = TTLParams(urgent=0, bulk=86400)
    options = SendOptions(ttl_params=ttl)
    for file in log_files:
        req = SendFileRequest(
            destination="ground",
            topic=oort_topic_logs,
            filepath=file,
            options=options
            )
        agent.send_file(req)

    # the raw data may be much larger, but still useful if there is 
    # time to transmit it.  This data can be sent as "surplus" 
    ttl = TTLParams(urgent=0, bulk=0, surplus=(86400 * 7))
    options = SendOptions(ttl_params=ttl)
    req = SendFileRequest(
        destination="ground",
        topic=oort_topic_raw,
        filepath=raw_filename,
        options=options
        )
    agent.send_file(req)

```

On the satellite, the producer will typically perform some observation, 
post-process the recorded data (e.g., to remove duplicate data points or
run custom compression), and then send the file.

The OORT pipeline will use a general-purpose data compressor (e.g., gzip)
to compress data that is not already compressed; so it is not always
necessary; however a compressor that has been tuned to the specific data
is likely to perform better.

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
        print("No files available for retreival from {}".format(oort_topic_primary))
    else:
        for item in available_primary.files:
            print("Retrieving {id} ({path})".format(item.id, item.path))
            save_path = '/tmp/{}'.format(os.path.basename(item.path))
            req = RetrieveFileRequest(
                id=item.id,
                save_path=save_path)
            agent.retrieve_file(req)

            # the application can now upload or process the data as required
            upload_data(save_path, "processed")

    available_raw = agent.query_available_files(oort_topic_raw)
    if not available_raw.files:
        print("No files available for retreival from {}".format(oort_topic_raw))
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
