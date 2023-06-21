#!/bin/bash
# This script is used to reset the FPGA PCI device

device=$(lspci | grep Xilinx | cut -d ":" -f 1)
for i in $device
do
    echo /sys/bus/pci/devices/0000\:"$i"\:00.0/remove
    echo 
    echo 1 > /sys/bus/pci/devices/0000\:"$i"\:00.0/remove
done

sleep 1
echo 1 > /sys/bus/pci/rescan
lspci -vd 10ee:
exit