#!/bin/bash

/usr/sbin/sshd -p 2022 -D &
/oort-agent -w /workdir &
python /payload.py &
wait
