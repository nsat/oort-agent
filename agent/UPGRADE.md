## Upgrading the OORT agent

### Version 1.0

Upgrading from any previous version to version 1.0 of the OORT agent only
requires updating the oort-agent binary to the new version.  The agent
should be stopped when doing this to avoid permission errors or file 
corruption.

No configuration options have been changed in this version, so no startup
invocations need to be modified.

Steps:
1. Stop the agent.  
Example using monit: `monit stop oort-agent`
2. Remount filesystems r/w if necessary.
3. Copy new oort-agent binary over old binary.
Example: `cp /media/sd1/oort-agent/upgrades/oort-agent /usr/local/bin/oort-agent`
4. Remount filesystems r/o if necessary.
5. Restart the agent.
Example using monit: `monit start oort-agent`
6. Remove the upgrade binary.
Example: `rm /media/sd1/oort-agent/upgrades/oort-agent`
