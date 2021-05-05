**Version:** 1.0

# Example
## Initializing the Spire Linux Agent using the SDK

```python
from oort_sdk_client import SdkApi

agent = SdkApi()
```

```c
    #include "SdkAPI.h"

    apiClient_t *client;

    client = apiClient_create();

    /* Do Work */

    apiClient_free(client);
```

In this example we will be initializing the SDK object to communicate with the Spire Linux Agent.
