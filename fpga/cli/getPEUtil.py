import subprocess
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import sys
import threading
import locale
import argparse

NUM_PE = 8

MDMA_CSR_DPE_MON_OFFSET = 0x30100
MDMA_CSR_CLR_OFFSET     = 0x30200

MDMA_BITMAP = [1,0,0,0, 0,0,0,0]#[1,1,1,1, 1,1,1,1]
MDMA_BASE_ADDRS = [0x81200000, 0x83200000, 0x85200000, 0x87200000, 0x91200000, 0x93200000, 0x95200000, 0x97200000]

colors = sns.color_palette('hls',NUM_PE)

isBwaMem2 = False

def command(command):
    exitcode, out = subprocess.getstatusoutput(command)
    if (exitcode != 0):
        exit(-1)
    return out, exitcode

def rt_command(cmd, isExit=True):
    p = subprocess.Popen(cmd,
                        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    while p.poll() == None:
        out = p.stdout.readline()
        print(out, end='')
    p.kill()

def is_running_bwa_main():
    out = command('ps -aux | grep bwa')
    if isBwaMem2:
        if 'bwa-mem2 mem' in out:
            return False
    else:
        if 'bwa_main mem' in out:
            return False
    return True

def getPeUtils(log0, log1):
    utils=[]
    lines0 = log0.split('\n')
    actArr=[]
    restArr=[]
    for line in lines0:
        if "0x" in line:
            data = line.split('|')[1]
            curAct = (int(data[:11].replace('_',''),16))
            actArr.append(curAct)
            curRest = (int('0x'+data[12:].replace('_',''),16))
            restArr.append(curRest)

    lines1 = log1.split('\n')
    for line in lines1:
        if "0x" in line:
            data = line.split('|')[1]
            curAct = (int(data[:11].replace('_',''),16))
            curRest = (int('0x'+data[12:].replace('_',''),16))
            act = curAct - actArr.pop(0);
            rest = curRest - restArr.pop(0);
            total = act + rest
            if total == 0:
                util = 0
            else:
                util = act/total
            utils.append(util*100)
    return utils

def input_timer(prompt, timeout_sec):
    class Local:
        # check if timeout occured
        _timeout_occured = False

        def on_timeout(self, process):
            self._timeout_occured = True
            process.kill()
            # clear stdin buffer (for linux)
            # when some keys hit and timeout occured before enter key press,
            # that input text passed to next input().
            # remove stdin buffer.
            try:
                import termios
                termios.tcflush(sys.stdin, termios.TCIFLUSH)
            except ImportError:
                # windows, just exit
                pass

        def input_timer_main(self, prompt_in, timeout_sec_in):
            # print with no new line
            print(prompt_in, end="")

            # print prompt_in immediately
            sys.stdout.flush()

            # new python input process create.
            # and print it for pass stdout
            cmd = [sys.executable, '-c', 'print(input())']
            with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as proc:
                timer_proc = threading.Timer(timeout_sec_in, self.on_timeout, [proc])
                try:
                    # timer set
                    timer_proc.start()
                    stdout, stderr = proc.communicate()

                    # get stdout and trim new line character
                    result = stdout.decode(locale.getpreferredencoding()).strip("\r\n")
                finally:
                    # timeout clear
                    timer_proc.cancel()

            # timeout check
            if self._timeout_occured is True:
                # move the cursor to next line
                #print("")
                raise TimeoutError
            return result

    t = Local()
    return t.input_timer_main(prompt, timeout_sec)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--mem2', '-m2', action='store_true')
    args = parser.parse_args()
    isBwaMem2 = args.mem2

    df = pd.DataFrame({'Time':[],'Cluster':[],'PE':[],'Util':[]})
    print('Wait until the bwa_main mem is finished.... Press Any Key to Exit!!')
    while (is_running_bwa_main() == True):
        try:
            a = input_timer("", 1)
            break
        except TimeoutError as e:
            for idx, baseAddr in enumerate(MDMA_BASE_ADDRS):
                if MDMA_BITMAP[idx] == 1 :
                    monAddr = baseAddr + MDMA_CSR_DPE_MON_OFFSET
                    out0, exitcode = command("sudo ./cli.py dump 0x%lx 0x%lx 1"%(monAddr,monAddr+0x40))
                    out1, exitcode = command("sudo ./cli.py dump 0x%lx 0x%lx 1"%(monAddr,monAddr+0x40))
                    utils = getPeUtils(out0, out1)
                    for peNum, util in enumerate(utils):
                        df.loc[len(df), ['Time','Cluster','PE','Util']] = [(len(df)//NUM_PE)/10, idx, peNum, util]
                    resetAddr = baseAddr + MDMA_CSR_CLR_OFFSET
                    #out, exitcode = command("sudo ./cli.py writeqw 0x%lx 1"%(resetAddr))

    print('Parsing Log......')
    enableCluster = 0
    for clusterId, enable in enumerate(MDMA_BITMAP):
        if enable == 1:
            enableCluster = enableCluster + 1
            for pe in range(NUM_PE):
                clusterDF=df[df['Cluster']==clusterId]
                print('[CLUSTER %d][PE %d] Average = %.6f'%(clusterId,pe,clusterDF[clusterDF['PE']==pe]['Util'].mean()))

    print('Making Graph.......')
    plt.figure(figsize=(14,7))
    for clusterId, enable in enumerate(MDMA_BITMAP):
        if enable == 1:
            cnt0 = 2
            cnt1 = 4
            if enableCluster < 4 :
                cnt0 = 1
                cnt1 = enableCluster
            plt.subplot(cnt0,cnt1,clusterId+1)
            clusterDF=df[df['Cluster']==clusterId]
            x = clusterDF[clusterDF['PE']==0]['Time']
            ys = [clusterDF[clusterDF['PE']==i]['Util'] for i in range(NUM_PE)]

            for i, y in enumerate(ys):
                plt.scatter(x,y, color=colors[i], alpha=0.4, label = f'PE({i})')

            plt.title('Cluster%d'%clusterId)
            plt.xlabel('Time')
            plt.ylabel('Util')
            plt.ylim([0, 100])
    plt.legend(bbox_to_anchor=(1.05,1))
    ##plt.savefig('Utils.png')
    plt.show()