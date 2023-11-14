**Version:** 1.0

# Data Structures

## TTLParams

```
see SendFileRequest for usage example
```

Time-to-live (TTL) parameters for a sent item

### Members

Unless otherwise specified, all time values are in seconds.

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| urgent | int | TTL for urgent queue | 9000  (2.5 hours) |
| bulk | int | TTL for bulk queue | 43200 (12 hours) |
| surplus | int | TTL for bulk queue | 172800 (48 hours) |

## SendOptions
```
see SendFileRequest for usage example
```


Options to apply to a send request

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| TTLParams | TTLParams | time-to-live parameters | see [TTLParams](#ttlparams) |
| reliable | boolean | whether to send an item reliable, i.e., with retries | true |

## SendFileRequest

```c
#include "SdkAPI.h"

ttl_params_t *ttl;
send_options_t *send_options;
send_file_request_t *request;

/* for TTL Params */
int urgent = 9000;
int bulk = 43200;
int surplus = 172800;
/* for SendOptions */
int reliable = 1;
/* for SendFileRequest */
/* NB: all the strings are owned by the reqeust object and will be freed
   with it so they must be copied if they come from storage that cannot
   be freed, like static strings or argv.
 */

char *dest = strdup("ground");
char *topic = strdup("logs");
char *filename = strdup("/file/to/send");

ttl = ttl_params_create(urgent, bulk, surplus);
send_options = send_options_create(ttl, reliable);
request = send_file_request_create(dest, filename, topic, send_options);

/* ... use request ... */

send_file_request_free(request);  /* frees all child structures */
```

```python
from oort_sdk_client.models import (
    TTLParams, SendOptions, SendFileRequest
)


ttl = TTLParams(urgent=9000, bulk=43200, surplus=172800)
send_options = SendOptions(ttl_params=ttl, reliable=True)
request = SendFileRequest(destination='ground', topic='logs',
                          filepath='/file/to/send',
                          options=send_options))

# ... use request ...
```
A request to send a file via the Data Pipeline API

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| destination | string | the destination to send the file to | - |
| filepath | string | The source file path.  Must be absolute | - |
| topic | string | The pipeline topic to send the file to | - |
| options | SendOptions | The options to apply | see [SendOptions](#sendoptions) |

## SendFileResponse
```c

send_file_response_t *response;

/* create apiClient_t *client, send_file_request *request */

response = SdkAPI_sendFile(client, request);

if (response != NULL) {
    fprintf(stderr, "The file was sent with uuid %s\n", response->uuid);
    send_file_response_free(response);
}

```
```python

# create SdkApi agent, SendFileRequest request

response = agent.send_file(request)
print('The file was sent with uuid {}'.format(response.uuid))

```

The response received from a send file request.

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| UUID | string | the unique identifier for the file transfer | - |

## FileInfo

```
see RetrieveFileRequest for example
```

Information about the file and the transfer request.

### Members

Times are unix epoch times (i.e., seconds since Jan 1, 1970)

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| id | string | the UUID for the file transfer | - |
| path | string | the original path this file was sent as | - |
| size | integer | the original size of this file | - |
| modified | integer | the time this file was most recently modfied | - |
| created | integer | the time this file was created | - |
| crc32 | string | CRC-32 calculated for the file | - |

## AvailableFilesResponse
```
see RetrieveFileRequest for example
```

Response to available files query

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| files | FileInfo\[\] | list of the available files | - |
| overflow | boolean | true if there are mote files available than could be returned in a single call | - |

## RetrieveFileRequest
```c

available_files_response_t *avail;
file_info_t *finfo;
retrieve_file_request_t *request;

char *topic;

topic = strdup("mytopic");

avail = SdkAPI_queryAvailableFiles(client, topic);

char *destdir = "/destination/directory";

if (avail != NULL) {
    listEntry_t *item;
    fprintf(stderr, "Available files:\n");
    /* provided helper macros */
    list_ForEach(item, avail->files) {
        file_info_t *finfo = (file_info_t *)item->data;
        file_info_t *finfo2;
        char *destpath;
        size_t pathlen;
        fprintf(stderr, "%s: %s\n", finfo->id, finfo->path);
        pathlen = snprintf(NULL, 0, "%s/%s", destdir, basename(finfo->path));
        destpath = (char *)malloc(pathlen + 1);
        pathlen = snprintf(destpath, pathlen, "%s/%s", destdir, basename(finfo->path));
        request = retrieve_file_request_create(strdup(finfo->id), destpath);
        finfo2 = SdkAPI_retrieveFile(client, request);
        fprintf(stderr, "retrieved %s\n", finfo2->id);
        file_info_free(finfo2);
        file_info_free(finfo);
        retrieve_file_request_free(request);
    }
    available_files_response_free(avail);
}
```

```python
from oort_sdk_client import SdkApi
from oort_sdk_client.models import FileInfo, RetrieveFileRequest
 
available = agent.query_available_files(thetopic).files
if not available:
    print("No files available")
else:
    for f in available:
        print("Retrieving {}".format(f.id))
        req = RetrieveFileRequest(
            id=f.id,
            save_path="/tmp/{}".format(os.path.basename(f.path)))
        rfinfo = agent.retrieve_file(req)
        print("Retrieved {}".format(rfinfo.id))
```

Request to retrieve a file from the Data Pipeline API

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| id | string | the UUID for the file transfer | - |
| save_path| string | The absolute path for the file to be saved to. | - |

## ErrorResponse

Any error returned from the Data Pipeline API.

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| status | integer | status code for the error | - |
| message| string | The error message | - |
