## installer

This directory contains support files for building the install packages,
including the startup files for supported daemon managers (monit and systemd).  

The Makefile here will call make in the agent directory to create the
cross-compiled binaries to be included in the install packages.
