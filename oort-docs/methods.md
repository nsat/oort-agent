**Version:** 1.0

# Methods

## Instantiation

```python
from oort_sdk_client import SdkApi

agent = SdkApi()
```

```c
    #include "SdkAPI.h"

    apiClient_t *client;

    client = apiClient_create();

    /* ... */

    apiClient_free(client);
```

### SdkAPI

Create a new OORT API Client.

## SendFile

Sends a file from the payload to the ground.

```python
from oort_sdk_client.models import SendFileRequest, RetrieveFileRequest

req = SendFileRequest(
    destination="ground",
    topic="my-topic",
    filepath="/path/to/file")
resp = agent.send_file(req)

print("File sent successfully, UUID is {}".format(resp.uuid)))
```

```c
    send_file_request_t *req;
    send_options_t *send_opt;
    send_file_response_t *resp;
    ttl_params_t *ttl;

    ttl = ttl_params_create(urgent, bulk, surplus);
    send_opt = send_options_create(ttl, reliable);

    req = send_file_request_create(dest, filename, topic, send_opt);

    resp = SdkAPI_sendFile(client, req);
    if (resp != NULL) {
        fprintf(stderr, "resp uuid = %s", resp->uuid);
        send_file_response_free(resp);
    }

    send_file_request_free(req);
```

Send a file via the API.

### Arguments

| Type | Description |
| ---- | ----------- |
| [SendFileRequest](#sendfilerequest) | SendFileRequest Object |

### Return value

| Type | Description |
| ---- | ----------- |
| [SendFileResponse](#sendfileresponse) | Contains the UUID assigned for this file transfer |

## QueryAvailableFiles

Queries files that have been uplinked from the ground to the payload

```python
from oort_sdk_client import SdkApi

agent = SdkApi()

topic = "my-topic"

available = agent.query_available_files(topic)

for item in available.files:
    print("Available file: {item.id} -- {item.path}".format(item=item))
```

```c
    char *topic = NULL;
    apiClient_t *client;
    available_files_response_t *resp;

    topic = strdup(argv[1]);
    client = apiClient_create();

    resp = SdkAPI_queryAvailableFiles(client, topic);
    if (resp != NULL) {
        listEntry_t *item;
        fprintf(stderr, "Available files:\n");
        list_ForEach(item, resp->files) {
            file_info_t *finfo = (file_info_t *)item->data;
            fprintf(stderr, "%s: %s\n", finfo->id, finfo->path);
        }
        available_files_response_free(resp);
    }

    apiClient_free(client);
```

Query the OORT agent for any files ready to be retrieved.

### Arguments

| Type | Description |
| ---- | ----------- |
| string | The topic to check for available files. |


### Return value

| Type | Description |
| ---- | ----------- |
| [AvailableFilesResponse](#availablefilesresponse) | AvailableFilesResponse Object |


## RetrieveFile

Retrieve a file returned from `QueryAvailableFiles`

```python
import os.path

from oort_sdk_client import SdkApi
from oort_sdk_client.models import RetrieveFileRequest

agent = SdkApi()

topic = "my-topic"

available = agent.query_available_files(topic)
if not available.files:
    print("No files available for retreival")
else:
    for item in available.files:
        print("Retrieving {id} ({path})".format(item.id, item.path))
        req = RetrieveFileRequest(
            id=item.id,
            save_path='/tmp/{}'.format(os.path.basename(item.path)))
        agent.retrieve_file(req)
```

```c
    char *filename = NULL, *id = NULL;
    apiClient_t *client;
    retrieve_file_request_t *req;
    file_info_t *finfo;

    if (argc != 3) {
        usage(argv[0]);
    }
    id = strdup(argv[1]);
    filename = strdup(argv[2]);

    client = apiClient_create();

    req = retrieve_file_request_create(id, filename);

    finfo = SdkAPI_retrieveFile(client, req);
    if (finfo != NULL) {
        fprintf(stderr, "retrieved id = %s to %s",
               req->id, finfo->path);
        file_info_free(finfo);
    }

    retrieve_file_request_free(req);
    apiClient_free(client);
```

Retrieve an available file from the OORT agent.

### Arguments

| Type | Description |
| ---- | ----------- |
| [RetrieveFileRequest](#retrievefilerequest) | RetrieveFileRequest Object |



### Return value

| Type | Description |
| ---- | ----------- |
| [FileInfo](#fileinfo) | Details about the file retrieved |
