**Version:** 1.0

# Example
## Payload Code

```python
import time
from oort_sdk_client.api.sdk_api import SdkApi
from oort_sdk_client.models import (
  AdcsCommandRequest, AdcsTarget, AdcsCommandResponse, AdcsHk, AdcsResponse, TfrsResponse
)

agent = SdkApi()

start = time.time()

tfrs_data = agent.get_tfrs()
# if the tfrs data isn't current, there may be some issue with the data feeds
if tfrs_data.age > 5:
    print("TFRS data is too old, aborting.")
    exit(1)

time_diff = tfrs_data.utc_time - start
print("TFRS time is {}, local time is {} difference {}".format(tfrs_data.utc_time, start, time_diff))

# get current ADCS data
adcs_data = agent.get_adcs()

target_area = {"lat": (-40, 0), "lon": (115,179)}

# if the current location is not over the target area, quit
print("Latitude: {:.2f}  Longitude {:.2f}".format(adcs_data.hk.lat_deg, adcs_data.hk.lon_deg))
if not ((target_area['lat'][0] <= adcs_data.hk.lat_deg <= target_area['lat'][1]) and 
        (target_area['lon'][0] <= adcs_data.hk.lon_deg <= target_area['lon'][1])):
    print("Satellite is outside the requested area.  Exiting.")
    exit(1)

print("Satellite has reached the designated area.  Requesting NADIR pointing for GOPRO")
command = AdcsCommandRequest(command="NADIR", aperture="GOPRO")

result = agent.command_adcs(command)

if result.status != "OK":
    print("ADCS command failed: {}".format(result.reason))
    exit(1)

feedback_mode = result.mode
print("ADCS switching to mode {}".format(feedback_mode))

mode_set = False

# poll ADCS, waiting up to 180 seconds for the pointing error to be less than 0.5 degrees
# abort if the mode is switched out beneath us or if we leave the target area
while True:
    current_tfrs_data = agent.get_tfrs()
    adcs_data = agent.get_adcs()
    if not ((target_area['lat'][0] <= adcs_data.hk.lat_deg <= target_area['lat'][1]) and 
            (target_area['lon'][0] <= adcs_data.hk.lon_deg <= target_area['lon'][1])):
        print("Satellite has left the area.  Exiting.")
        agent.command_adcs(AdcsCommandRequest("IDLE"))
        exit(1)

    if current_tfrs_data.utc_time - tfrs_data.utc_time > 180:
        print("Command taking too long.  Exiting.")
        agent.command_adcs(AdcsCommandRequest("IDLE"))
        exit(1)

    if mode_set:
        if adcs_data.hk.acs_mode_active != feedback_mode:
            print("ADCS command has been preempted ({}).  Exiting.".format(adcs_data.hk.acs_mode_active))
            agent.command_adcs(AdcsCommandRequest("IDLE"))
            exit(1)
    else:
        if adcs_data.hk.acs_mode_active != feedback_mode:
            print("ADCS mode request has not been updated, waiting")
            time.sleep(1)
            continue
        else:
            mode_set = True

    print("Pointing error: {:.2f} degrees".format(adcs_data.hk.control_error_angle_deg))
    if adcs_data.hk.control_error_angle_deg < 0.5:
        print("Desired pointing accuracy acheived")
        break

    # wait for time to pass
    time.sleep(1)
        
# At this point, the desired NADIR pointing accuracy has been achieved, so
# we can capture our images
from spire.gopro import GoProImager, ACTION, BURST_3
imager = GoProImager()
image_fn = imager.capture(mode=ACTION, best_shot_selector=BURST_3)
# tell ADCS that we're done
agent.command_adcs(AdcsCommandRequest("IDLE"))

from oort_sdk_client.models import SendFileRequest, TTLParams, SendOptions
filerequest = SendFileRequest(
    destination="ground", filepath=image_fn, topic="gopro",
    options=SendOptions(TTLParams(urgent=1800, bulk=86400*14)))
send_response = agent.send_file(filerequest)
print("File sent with UUID {}".format(send_response.uuid))

exit(0)
```
