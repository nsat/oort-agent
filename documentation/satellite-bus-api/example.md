# Example Usage

## Instantiation

```python
import time
from oort_sdk_client.api.sdk_api import SdkApi
from oort_sdk_client.models import (
  AdcsCommandRequest, AdcsTarget, AdcsCommandResponse, AdcsHk,
  AdcsResponse, TfrsResponse
)

agent = SdkApi()
```

The SDK must be initialized before using it.  All ADCS API operations
performed through methods on the `SdkApi` object.

The following examples assume this code is present.

## TFRS 

*TFRS (Time-Frequency Reference System)* provides a highly accurate
clock and position reading.

### Age check

```python
start = time.time()

tfrs_data = agent.get_tfrs()
# if the tfrs data isn't current, there may be some issue with
# the data feeds
if tfrs_data.age > 5:
    print("TFRS data is too old, aborting.")
    exit(1)

time_diff = tfrs_data.utc_time - start
print("TFRS time is {}, local time is {} difference {}"
     .format(tfrs_data.utc_time, start, time_diff))
print("TFRS ECEF position is "
      "{t.ecef_pos_x}, {t.ecef_pos_y}, {t.ecef_pos_z}"
     .format(t=tfrs_data))
```

TFRS data is typically updated once per second.  The `age` field indicates
how long it has been since the Agent has received an update from the TFRS.  It
is not uncommon or a serious error for an update to occasionally be lost, but
multiple updates getting lost may indicate a larger issue.

If the TFRS data is more than a few seconds old, discretion is advised before 
relying on it.

The code here checks the TFRS timestamp against the system clock and
displays the difference, as well as displaying the position reported.

## ADCS

*ADCS (Attitude Determination And Control System)* is a satellite subsystem that provides accurate 
attitude information about the spacecraft as well as a mechanism to re-orient the spacecraft.  

The ADCS API provided by Spire Space Services allows customer payloads to access current attitude 
information and, if applicable, the ability to maneuver the spacecraft.
In order for ADCS commands to take effect, a 
[LEASE_ADCS](/tasking-api-docs/index.html#lease_adcs) must have been
scheduled.   Without this, the payload commands will never be acted on.

### Reading current ADCS data

```python
adcs_data = agent.get_adcs()

target_area = {"lat": (-40, -30), "lon": (140,170)}

# if the current location is not over the target area, quit
print("Latitude: {:.2f}  Longitude {:.2f}"
      .format(adcs_data.hk.lat_deg, adcs_data.hk.lon_deg))
if not ((target_area['lat'][0] <= adcs_data.hk.lat_deg <= target_area['lat'][1]) and 
        (target_area['lon'][0] <= adcs_data.hk.lon_deg <= target_area['lon'][1])):
    print("Satellite is outside the requested area.  Exiting.")
    exit(1)
```

Using the `get_adcs` interface, the spacecraft surface position (i.e., the 
position *directly beneath* the satellite) can be obtained.  This can be used
for annotating data collections, or to check if the spacecraft is within 
specified geographic boundaries.

In this example, if the current position is not between 140 and 170 degrees east
longitude and between 30 and 40 degrees south latitude, the job will exit.

### Commanding a spacecraft maneuver

```python
command = AdcsCommandRequest(command="NADIR", aperture="GOPRO")

command_result = agent.command_adcs(command)

if command_result.status != "OK":
    print("ADCS command failed: {}".format(command_result.reason))
    exit(1)
else:
    feedback_mode = result.mode
    print("ADCS switching to mode {}".format(feedback_mode))
    mode_set = False  # see below
```

A maneuver can be requested using the `command_adcs` method.  
A maneuver is always performed relative to a named aperture,
which is an antenna, imager, or other instrument that needs 
to be pointed in the specified direction.

The result should always be checked to ensure that 
there were no errors in the request itself, and that the 
command was acknowledged and acted on by the control systems.

In addition, when the command is successful, the result will
indicate the physical command mode that the control system
will use.  This command mode can be used for validating that 
the requested operation is the current in progress operation. (see [below](#control-override))

In this example, the satellite is commanded to `NADIR`
orientation relative to the `GOPRO` aperture.  Stated
differently, "point the camera straight down."

Refer to your ICD for valid aperture names.


### Maneuver tracking

```python
while True:
    adcs_data = agent.get_adcs()

    print("Pointing error: {:.2f} degrees".format(
          adcs_data.hk.control_error_angle_deg))
    if adcs_data.hk.control_error_angle_deg < 0.5:
        print("Desired pointing accuracy achieved")
        break
    # wait for time to pass
    time.sleep(1)
```

When a maneuver is commanded, the spacecraft orientation cannot
change instantly.  The spacecraft's moment of inertia as well as its
previous orientation limits how quickly a requested orientation 
can be attained.   It is expected that it can take as much as 3 minutes
for a orientation change to complete.

It is recommended that after a command is issued, adcs is periodically
polled to wait for the requested orientation to be achieved;  
`control_error_angle_deg`, which is the difference between the current
 measured orientation and the requested orientation, is ideal to check.

The maneuver can be disrupted in certain cases;  see [below](#error-situations) 
for errors to check for.

### Performing an observation after a maneuver

```python
# NOTE: The `Imager` interface is used here for illustration
# only.  Refer to your ICD for the available interfaces.
from spire.imager import Imager, ACTION, BURST_3
imager = Imager()
image = imager.capture(mode=ACTION, best_shot_selector=BURST_3)
adcs_data = agent.get_adcs()
image.annotate(pos=adcs_data.r_ecef, orientation=adcs_data.qcf)
```

Once the desired orientation has been attained, payload observations
can be performed.  Measurements from ADCS may be used to annotate the
observations, so that they can be accurately matched up to earth
positions in post-processing.

**NOTE:** The `Imager` interface is used here for illustration
only.  Refer to your ICD for the available interfaces.

### Clean up and deliver

```python
agent.command_adcs(AdcsCommandRequest("IDLE"))

from oort_sdk_client.models import SendFileRequest, TTLParams, SendOptions
filerequest = SendFileRequest(
    destination="ground", filepath=image.get_file(), topic="gopro",
    options=SendOptions(TTLParams(urgent=1800, bulk=86400*14)))
send_response = agent.send_file(filerequest)
print("File sent with UUID {}".format(send_response.uuid))

exit(0)
```

After observations have been completed, ADCS can be commanded back to `IDLE`
mode.  This is not required and will be done automatically at the end of 
a payload window, but is suggested as an optimization of spacecraft activity.

Any created files to be sent to the ground can be transferred using the 
[data pipeline API](../data-pipeline-api/), which uses the same `SdkApi`
interface as an entry point.

### Error Situations

A maneuver can be interrupted for various reasons.  It is recommended to 
check for these situations to be able to take appropriate recovery
actions and/or log the situations for later analysis.

These error checks should be included in the [Maneuver tracking](#maneuver-tracking)
loop above.

#### Satellite moves out of position

```python
    adcs_data = agent.get_adcs()
    if not ((target_area['lat'][0] <= adcs_data.hk.lat_deg <= target_area['lat'][1]) and 
            (target_area['lon'][0] <= adcs_data.hk.lon_deg <= target_area['lon'][1])):
        print("Satellite has left the area.  Exiting.")
        agent.command_adcs(AdcsCommandRequest("IDLE"))
        exit(1)
```

The satellite is moving around the earth at close to 8km/s.  The satellite may 
leave the target area before attaining the desired orientation.   This case can
be monitored by checking that the `lat_deg` and `lon_deg` values remain within their intended bounds.

This example checks the same condition tested at the 
[beginning of the operation](#reading-current-adcs-data).

#### Maneuver taking too long

```python
    current_tfrs_data = agent.get_tfrs()
    if current_tfrs_data.utc_time - tfrs_data.utc_time > 180:
        print("Command taking too long.  Exiting.")
        agent.command_adcs(AdcsCommandRequest("IDLE"))
        exit(1)
```

The satellite does take time to physically change orientation. It is recommended 
to wait up to 3 minutes while polling.  If the desired state is not reached within this time frame, 
we recommend aborting the operation and logging appropriate information to identify the problem.

Note the reuse of `tfrs_data` obtained [above](#age-check).

#### Control override

```python
    if mode_set:
        if adcs_data.hk.acs_mode_active != feedback_mode:
            print("ADCS command has been preempted ({}).  Exiting."
                  .format(adcs_data.hk.acs_mode_active))
            agent.command_adcs(AdcsCommandRequest("IDLE"))
            exit(1)
    else:
        if adcs_data.hk.acs_mode_active != feedback_mode:
            print("ADCS mode request has not been updated, waiting")
            time.sleep(1)
            continue
        else:
            mode_set = True
```

The satellite control systems may determine that another maneuver
has a higher priority, for example to address a spacecraft fault.
This can usually be detected by the active mode being changed while
waiting for a maneuver to complete.

Is is important to note that due to the data feed mechanism the command
mode reported in the `get_adcs` result will lag behind a successful command 
by a few seconds, and the command mode being different immediately afterward
is expected.  It is only an override situation if the command mode switches
away from the physical mode indicated in the command result *after* it has 
already switched *into* that physical mode. 
