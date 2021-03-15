## spec

The OpenAPI specification for the OORT Agent.  These are used to auto-generate
framework code for the agent itself and the the language-specific client SDK
packages.

There are two API descriptions here, the SDK api and the collector api.

### oort-sdk-api.yml
The SDK api is the public, stable api.  It has the methods for clients to send
or retrieve files and other supported operations.  Any methods defined here
have an emphasis on maximum compatibility.

### oort-collector-api.yml
The collector api is for the internal use of the oort collector only.  No
guarantees are mode or implied about the compatibility of these methods
between versions.
