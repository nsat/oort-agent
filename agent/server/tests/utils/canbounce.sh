#!/bin/sh
while true; do
  echo sleeping...
  sleep 5
  echo turning off can0
  sudo ip link set can0 down
  echo sleeping...
  sleep 2
  echo turning on can0
  sudo ip link set can0 up
done
