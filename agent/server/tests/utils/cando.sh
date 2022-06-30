#!/bin/sh
sudo ip link add dev can0 type vcan
sudo ip link set can0 up
