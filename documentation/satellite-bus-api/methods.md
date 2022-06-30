# Reference - Methods 

## Instantiation
The client needs to be instantiated before it is used.

```python
from oort_sdk_client.api.sdk_api import SdkApi

agent = SdkApi()
```
**SdkAPI**

Create a new Spire Linux Agent Client.

## TfrsGet

```python
from oort_sdk_client.models import TfrsResponse

resp = agent.get_tfrs()

print("TFRS time reading is {}".format(resp.utc_time))
```

Gets the most recent time and position reading from TFRS.

**Return value**

| Type | Description |
| ---- | ----------- |
| [TfrsResponse](#tfrsresponse) | Contains the TFRS reading |

## AdcsGet

```python
from oort_sdk_client.models import AdcsResponse

resp = agent.get_adcs()

print("ADCS lat, lon is {}, {}".format(resp.hk.lat_deg, resp.hk.lon_deg))
```

Gets the most recent spacecraft attitude reading from ADCS.

**Return value**

| Type | Description |
| ---- | ----------- |
| [AdcsResponse](#adcsresponse) | AdcsResponse Object |


## AdcsCommand

```python
from oort_sdk_client.models import AdcsCommandRequest

request = AdcsCommandRequest(
    command="NADIR", aperture="IPI")

response = agent.command_adcs(request)
print("Response status: {}".format(response.status))
if response.status != "OK":
    print("Reason: {}".format(response.reason))
else:
    print("Command mode: {}".format(response.mode))

```

Send a command to ADCS

**Arguments**

| Type | Description |
| ---- | ----------- |
| [AdcsCommandRequest](#adcscommandrequest) | AdcsCommandRequest Object |


**Return value**

| Type | Description |
| ---- | ----------- |
| [AdcsCommandResponse](#adcscommandresponse) | Result of the command |
