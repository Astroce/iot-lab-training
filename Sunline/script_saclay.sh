#!/bin/bash

iotlab-experiment submit -n "Saclay" -d 10 -l 2,site=saclay+archi=st-lrwan1:sx1276

iotlab-experiment wait --timeout 30 --cancel-on-timeout

iotlab-experiment --jmespath="items[*].network_address | sort(@)" get --nodes

make BOARD=b-l072z-lrwan1 IOTLAB_NODES=2 -C ../Sunline iotlab-flash

