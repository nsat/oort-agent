**Version:** 1.0

# Example
## Payload Code

```python
from oort_sdk_client import SdkApi
from oort_sdk_client.models import (
    SendFileRequest, SendOptions, TTLParams
)

from custom_application import (
    observe, process, save_to_file, create_logs
)

# these topic names will be provided by the Spire Constellation Ops team
topic_primary = "custom-application"
topic_logs = "custom-application-logs"
topic_raw = "custom-application-raw"

agent = SdkApi()

while True:
    raw_observation = observe()

    # on-board processing may be done to extract the most important data
    processed_observation = process(raw_observation)

    raw_filename = save_to_file(raw_observation)
    processed_filename = save_to_file(processed_observation)

    # send the important processed data with default options
    req = SendFileRequest(
        destination="ground",
        topic=topic_primary,
        filepath=processed_filename,
        options=SendOptions())
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
            topic=topic_logs,
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
        topic=topic_raw,
        filepath=raw_filename,
        options=options
        )
    agent.send_file(req)

```

On the satellite, the producer will typically perform some observation, 
post-process the recorded data (e.g., to remove duplicate data points or
run custom compression), and then send the file.

The data pipeline will use a general-purpose data compressor (e.g., gzip)
to compress data that is not already compressed; so it is not always
necessary; however a compressor that has been tuned to the specific data
is likely to perform better.
