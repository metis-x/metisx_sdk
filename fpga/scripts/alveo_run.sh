#!/bin/bash
NUM_FPGA=$1
source sourceme
sudo ./reset_pcie.sh
sudo ./load_driver.sh 4
sudo ./init_ddr.sh $NUM_FPGA
