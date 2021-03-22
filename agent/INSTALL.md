# OORT Agent

## Install Guidelines

The OORT Agent needs to be started up at system boot
time.  Three systems are supported for this,
`monit`, `systemd` and `daemontools`.

## Common Requirements

### Architecture Support

The OORT Agent is currently supported for x86-64 (for ground station usage) 
and armv7a / armv8 (for satellite payloads) architectures.   
The binary size on disk is < 500Kb, and the runtime memory usage should stay below 5Mb.
These numbers apply to all architectures, with the actual numbers tending to be slightly
lower on the 32-bit ARM builds.  

### Binary Location

The OORT Agent is installed as an executable binary, so it must be installed on a filesystem 
mounted to permit execution of binaries.

The OORT Agent will be upgraded from time to time, so it must be installed in a location writable
by maintenance tasks (i.e., root).

### Working Directory

The OORT Agent requires its own private working directory for storage of files in transit.
This directory should be located on nonvolatile storage, so that it persists over a system reboot.
This directory needs sufficient storage space to hold all file transfers until they can be picked
up by the OORT Collector.  There is no default for the working directory, it must be specified with
the `-w` command line setting.

The preferred location for the working directory is to be on the same filesystem as the source
files to be transferred.  While this is not a requirement, this setup will allow files to be
handed off to the Agent more efficiently, and allows free space checks to be skipped.

### Free Space

When the agent working directory is located on a separate filesystem from the source files, 
a check for free space is done before the transfer.  By default, the check ensures that 20% of the
disk space will be free after the transfer.  The free space requirement is adjustable with
the `-m` command line setting.

### Cleanup

Files in the OORT Agent's working space are not expected to stay there very long, only until
they can get picked up by the collector (for sending) or by the payload application (for
receiving).  In most cases this will be within a few hours.   However, as a safeguard against
the disk filling up, the Agent periodically runs a cleanup job and removes any files which have
a modification time earlier than a specified threshold.  

By default, the cleanup job runs once an hour and removes files which are more than a day old.
These values can be adjusted with the `-i` command line flag for the cleanup interval,
and the `-t` flag for the cleanup threshold.

### Process user

The OORT Agent moves files around on behalf of SDK clients.  To perform the necessary 
file operations, it must operate as a user with appropriate permissions to read, write, 
and delete files.  This generally means that it must run either as the same unix user 
as the SDK clients (e.g., `spire`), or as root.  

In addition, the OORT Collector needs remote access to the payload board where the Agent is
running.  To support this, any board running the OORT Agent must have an SSH daemon installed
and running, the process user must have the Collector's public key in its `authorized_keys` 
file, and the `rsync` and `scp` programs must be installed.

### Logging

The OORT Agent writes some informational messages to track progress.  These messages default
to being written to its standard error, where they can be captured by another program for
logging to disk.  Alternately, the messages can be logged to syslog;  this behavior can be 
enabled with the `-s` command line flag with a syslog identifier to be used.

## Monit Setup

The `monit` setup consits of two files:
1. monit config file: `/etc/monit/conf.d/monit.agent`  
This file generally shouldn't require any customization.

2. init.d start/stop file: `/etc/init.d/oort-agent`  
Any setting changes are added in this file.

Examples of these files can be found in `installer/install/monit`.

## Daemontools Setup

The `daemontools` setup requires a single file, `/service/oort-agent/run`.
Any setting changes are made in this file.

Examples of these files can be found in `support/daemontools`.

## Systemd Setup

The `systemd` setup requires a single file, `installer/install/systemd/system/oort-agent.service`.


## Install Steps

1. Download the oort-agent archive for the appropriate architecture to a temporary
location on the target system.
2. Extract the archive.
3. Modify the startup scripts to be used in the archive to point to the appropriate 
locations for the installed binary and working directory, as well as any other options 
that are desired to be different from the defaiults.
4. If needed, remount filesystems as writable.
5. Copy oort-agent binary to its target location.
6. Copy the appropriate startup scripts to their target locations.
7. If filesystems were remounted writeable, remount then as read-only.
8. Delete the install archive and extracted files.

After this, the oort-agent should automatically be started by the process monitor
being used, and should be automatically started in the future when the system reboots.
