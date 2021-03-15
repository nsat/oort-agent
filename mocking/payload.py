import logging
from oort_sdk_client import SdkApi
from oort_sdk_client.models import SendFileRequest
import time
import glob

logging.basicConfig()
log = logging.getLogger(None)

sdk = SdkApi()

while True:
    time.sleep(1)
    log.info("scanning dropbox")
    for file in glob.glob("/dropbox/*"):
        req = SendFileRequest(filepath=file, topic="default", destination="ground")
        log.info("sending %s", file)
        sdk.send_file(req)
