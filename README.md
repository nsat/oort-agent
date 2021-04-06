** Spire Export Controlled **  |
------------------------------ |

## OORT Agent

This is the public repository for the OORT Agent.

## What is the OORT Agent?

The OORT Agent is a standalone server that is the endpoint of the OORT pipeline.
It currently provides basic functions to send and receive files, and to support
the rest of the pipeline.

The Agent is intended to be an always-available service.  To support this goal,
it requires only a small amount of resources from the host system.

## How to build the OORT Agent

### Native build

The agent can be built for the current platform using `make`:

``` bash
$ cd agent
$ make
```

The compiled binary can be found in `agent/build/oort-server`.

### Cross-compiling

The OORT agent can be built for several different architectures.  Building for
architectures other than native require a cross-compiler.  These cross-compilers
are packaged in docker images, and can be built from the `builder` subdirectory.

``` bash
$ cd builder
$ make armv7a arm8
```

Note that these cross-compilers take significant time (several hours) to build.  Be careful
cleaning up images unless you wish to repeat these lengthy builds!

Once the cross-compiler dockers are available, the agent itself can be built.

``` bash
$ cd agent
$ make cross ARCH=armv7a
```

This will build the agent for the `armv7a` architecture, suitable for the OBC and most
other satellite payloads.  The compiled binary will be found in `agent/build-cross-armv7a/oort-server`

## How to run the OORT Agent

The OORT Agent is expected to run at system startup and be available at any time after that.
In testing settings, it can be started normally from the command line:  

`oort-agent -w /tmp/workdir`

The agent performs several checks at startup to minimize surprises later.  Among these
is checking the ownership and permissions on the working directory, so if those are 
incorrect the agent will refuse to start.

### Configuration options

All configuration options are specified on the command line.  Running with `-h` will
give a brief description of the options, reproduced here.

```
oort-agent -w workdir
 [-t cleanup-timeout] [-i cleanup-interval] [-f config-file]
 [-s ident] [-m minfree] [-p port]
 workdir - base working directory; must be writable
 cleanup-timeout - age in seconds after which files can be deleted
 cleanup-interval - how frequently in seconds to run the cleanup task
 config-file - file to use for setting config values (not implemented)
 ident - identifier for syslog (default is to log to stderr)
 minfree - minimum free space to maintain; either ddM or dd%
 port - tcp port to listen on

Defaults: 
 cleanup-timeout = 86400  cleanup-interval = 3600
 minfree = 20%
 port = 2005
 ```

## How to install the OORT Agent

The OORT Agent is installed by copying the binary to the target location, and configuring
the system startup scripts to run it from that location.  

Full details can be found in [INSTALL.md](./agent/INSTALL.md).

## How to upgrade the OORT Agent

The OORT Agent is upgraded by copying the new binary over the existing one.  Configutation
options are the same or compatible, so changes to the startup scripts are not necessary,
other than to configure previously unavailable functionality.

In the usual case, new oort-agent binaries will be uploaded to the host by the OORT collector.  
If it has been configured, the collector will kick off the upgrade install as well, requiring
no additional manual steps.

Full details can be found in [UPGRADE.md](./agent/UPGRADE.md).
