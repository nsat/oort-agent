**Version:** 1.0

# Data Structures

## TTLParams

```c
#include "SdkAPI.h"

ttl_params_t *ttl;

int urgent = 9000;
int bulk = 43200;
int surplus = 172800;

ttl = ttl_params_create(urgent, bulk, surplus);
```

```python
# insert example here
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

Options to apply to a send request

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| TTLParams | TTLParams | time-to-live parameters | see #TTLParams |
| reliable | boolean | whether to send an item reliable, i.e., with retries | true |

## SendFileRequest

A request to send a file via the OORT Pipiline

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| destination | string | the destination to send the file to | - |
| filepath | string | The source file path.  Must be absolute | - |
| topic | string | The pipeline topic to send the file to | - |
| options | SendOptions | The options to apply | see [SendOptions](#sendoptions) |

## SendFileResponse

The response received from a send file request.

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| UUID | string | the unique identifier for the file transfer | - |

## FileInfo

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

Response to available files query

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| files | FileInfo\[\] | list of the available files | - |
| overflow | boolean | true if there are mote files available than could be returned in a single call | - |

## RetrieveFileRequest

Request to retrieve a file from the OORT pipeline

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| id | string | the UUID for the file transfer | - |
| save_path| string | The absolute path for the file to be saved to. | - |

## ErrorResponse

Any error returned from the OORT agent.

### Members

| Name | Type | Description | Default |
| ---- | ---- | ----------- | ------- |
| status | integer | status code for the error | - |
| message| string | The error message | - |
