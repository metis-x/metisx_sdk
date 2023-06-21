#!/bin/bash
# set -x
START=0
END=$1
END=$((END-1))
echo "Init DDR 32GB"
for i in $(seq 0 $END)
do
    echo "tools/dma_to_device -d /dev/xdma"$i"_h2c_0 -i 1"
    tools/dma_to_device -d /dev/xdma"$i"_h2c_0 -i 1
done
echo "DONE"
