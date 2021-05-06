# Spire Linux Agent Introduction

The Spire Linux Agent is a daemon that runs on a Space Services customer's payload and provides a set of APIs that allow
the payload to interface with the rest of the satellite bus.

Currently Supported APIs include:

* [Data Pipeline API](../data-pipeline-docs/index.html) - Allows users to send and receive files via ISLs or ground based contacts.

Note: The Spire Linux Agent was formerly know as the OORT Agent.  The OORT name is still referenced in SDK modules and process names.

# Components

In addition to the Spire Linux Agent code, Spire provides SDKs to facilitate communication between payload 
developer applications and the Spire Linux Agent daemon.

* [Spire Linux Agent Daemon](https://github.com/nsat/oort-agent)

SDKs

* [Python SDK](https://github.com/nsat/oort-sdk-python)
* [C SDK](https://github.com/nsat/oort-sdk-c)

# How to build the Spire Linux Agent

For Payload in Space customers using Linux payloads, it is expected that the customers compile and install the Spire Linux Agent
on their payload.  Additionally, the Spire Linux Agent should be started automatically upon boot of the payload. 

```shell
$ git clone https://github.com/nsat/oort-agent
$ cd oort-agent
```

## Native build

The agent can be built for the current platform using `make`:

```shell
$ cd agent
$ make
```

The compiled binary can be found in `agent/build/oort-server`.

## Cross-compiling

### Building cross-compiler

The agent can be built for several different architectures.  Building for
architectures other than native require a cross-compiler.  These cross-compilers
are packaged in docker images, and can be built from the `builder` subdirectory.

```shell
$ cd builder
$ make armv7a arm8
```

Note that these cross-compilers take significant time (several hours) to build.  Be careful
cleaning up images unless you wish to repeat these lengthy builds!

### Cross-compiling Agent

Once the cross-compiler dockers are available, the agent itself can be built.

```shell
$ cd agent
$ make cross ARCH=armv7a
```

This will build the agent for the `armv7a` architecture, suitable for the OBC and most
other satellite payloads.  The compiled binary will be found in
`agent/build-cross-armv7a/oort-server`
