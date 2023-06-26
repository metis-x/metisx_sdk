#!/usr/bin/env python3
import os
from pydoc import doc
import subprocess
import argparse
import time
import asyncio
from datetime import datetime

VERBOSE = 1

def Command(command):
    if (VERBOSE >= 1):
        print(f'COMMAND: {command}')
    
    exitcode, out = subprocess.getstatusoutput(command)
    print(out)

    if (exitcode != 0):
        exit(-1)
    return out, exitcode

syncFlag = False
def progressCommand(command):
    global syncFlag
    syncFlag = True

    async def asyncCommandRun(command):
        global syncFlag
        loop = asyncio.get_event_loop()  # 이벤트 루프 객체 얻기
        await loop.run_in_executor(None, Command, command) # 동기함수를 비동기로 호출
        syncFlag = False

    async def progress():
        global syncFlag
        i = 0
        while(syncFlag):
            await asyncio.sleep(0.1)
            i = (i + 1) % 4

            if (i == 0): print("\r/", end='')        
            elif (i == 1): print("\r-", end='')
            elif (i == 2): print("\r\\", end='')
            elif (i == 3): print("\r|", end='')

        print()

    async def run(command):
        await asyncio.gather(
            asyncCommandRun(command),
            progress(),
        )

    asyncio.run(run(command))

def get_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('--target', '-t', help='Path of FPGA alveo image', dest='target')
    parser.set_defaults(target='')

    target = parser.parse_args().target
    init = True
    no_exit = True

    if (target == ''):
        print("Please enter the path of FPGA image (--target)")
        exit(-1)
    
    return target, init, no_exit

def get_num_fpga():
    numFpga_proc, exitcode = Command("lspci | grep '00.0 Processing accelerators: Xilinx' | wc -l")
    numFpga_proc = int(numFpga_proc)
    numFpga_serial, exitcode = Command("lspci | grep '00.0 Serial controller: Xilinx' | wc -l")
    numFpga_serial = int(numFpga_serial)

    return numFpga_proc + numFpga_serial

def make_sourceme():
    with open('scripts/sourceme', "w") as file:
        file.write(f'#!/bin/bash\n')
        file.write(f'export METISX_API_PATH={os.path.dirname(os.path.abspath(__file__))}/metisx_api/\n')
        file.write(f'alias cli="sudo python3 {os.path.dirname(os.path.abspath(__file__))}/cli/cli.py"\n')
        file.write(f'alias watch="watch -c -d -n 0.1 "\n')
        file.write(f'alias cdperf="cd {os.path.dirname(os.path.abspath(__file__))}/cli/perf_monitor"\n')

def make_tcl(imagePath, numFpga):
    def findfile(name, path):
        for dirpath, dirname, filename in os.walk(path):
            for files in filename:
                if name in files:
                    return os.path.join(dirpath, files)

    bitfile = findfile(".bit", imagePath)
    xsafile = findfile(".xsa", imagePath)

    print("num FPGA : " + str(numFpga))

    with open('load_fpga_image.tcl', "w") as file:
        file.write("connect -url tcp:127.0.0.1:3121\n")
        for fpgaId in range(int(numFpga)):
            fpgaIndex = fpgaId * 4 + 1 
            file.write("targets " + str(fpgaIndex) +"\n")
            file.write("fpga "+bitfile+"\n")
            file.write("loadhw -hw "+xsafile+" -regs\n")
            file.write("configparams mdm-detect-bscan-mask 2\n")
            fpgaIndex += 2
            file.write("targets " + str(fpgaIndex) +"\n")
            file.write("rst -processor\n")
            file.write("after 3000\n")
            file.write(f"dow {os.path.dirname(os.path.abspath(__file__))}/microblaze/microblaze.elf\n")
            file.write("bpadd -addr &main\n")
            file.write("after 500\n")
            file.write("con\n")
            file.write("after 500\n")
            file.write("con\n")
            file.write("after 500\n")
    
    with open('kick_micro.tcl', "w") as file:
        file.write("connect -url tcp:127.0.0.1:3121\n")
        for fpgaId in range(int(numFpga)):
            fpgaIndex = fpgaId * 4 + 3
            file.write("targets " + str(fpgaIndex) +"\n")
            file.write("after 300\n")
            file.write("mwr 0xc1080100 0x0\n")
            file.write("after 300\n")

def main(target, init, no_exit):
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    start = time.time()

    numFpga = get_num_fpga()

    make_sourceme()

    make_tcl(target, numFpga)

    xdmaExist = subprocess.getoutput("lsmod | grep xdma")
    if xdmaExist != "":
        Command("sudo rmmod xdma")

    progressCommand("xsct -eval source load_fpga_image.tcl")

    progressCommand("cd scripts && ./alveo_run.sh " + str(numFpga))

    progressCommand("xsct -eval source kick_micro.tcl")

    end = time.time()

    print(f"TARGET:{target}")
    difftime = end - start
    print("=================================================")
    print(f"수행시간 : {int(difftime / 60)} min {difftime % 60:.5f} sec")
    print("=================================================")
    print("Build Completion Time : " + datetime.now().strftime('%Y.%m.%d - %H:%M:%S'))


if __name__ == '__main__':
    main(*get_arguments())