#!/bin/sh
set -ex
mkdir /media/sd1/oort-agent || true

cd `dirname $0`
monit stop oort-agent
rm -f /usr/local/bin/oort-agent
cp oort-agent /usr/local/bin/oort-agent
cp init.d/oort-agent /etc/init.d/oort-agent
cp monit/monit.agent /etc/monit.d/oort-agent.cfg

monit reload
monit start oort-agent
sleep 1
monit status oort-agent
