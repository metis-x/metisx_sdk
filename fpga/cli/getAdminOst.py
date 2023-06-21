import subprocess
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import time
import sys
import argparse

isBwaMem2 = False

def Command(command):
    exitcode, out = subprocess.getstatusoutput(command)
    if (exitcode != 0):
        exit(-1)

    return out

def is_running_bwa_main():
    out = Command('ps -aux | grep bwa')
    if isBwaMem2:
        if (out.find('bwa-mem2 mem') == -1):
            return False
    else:
        if (out.find('bwa_main mem') == -1):
            return False
    return True

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--mem2', '-m2', action='store_true')
    args = parser.parse_args()
    isBwaMem2 = args.mem2

    process = subprocess.Popen(["sh","logAdmin.sh"])
    print("Wait until the bwa_main mem is finished....")

    try:
        while (is_running_bwa_main() == True):
            print('.', end="")
            time.sleep(0.1)
            sys.stdout.flush()    
    except KeyboardInterrupt:
        pass

    process.kill()

    print('Parsing Log.......')
    log = open("admin_.log", 'r')
    df = pd.DataFrame({'Time':[],'OstQ':[]})
    while True:
        line = log.readline()
        if not line: break
        value = int(line.split('=')[1].split('\n')[0].split('_')[3], 16)
        # if value > 200:
        #     print('Check UnderFlow!! - %d'%value)
        # else:
        df.loc[len(df), ['Time','OstQ']] = [len(df)/10, value]
    log.close()
    print('Average = %.6f'%df['OstQ'].mean())
    print('Making Graph......')

    x = df['Time']
    y = df['OstQ']
    z = np.polyfit(x,y,1)
    p = np.poly1d(z)
    plt.scatter(x,y, color='skyblue', alpha=0.4)
    plt.plot(x,p(x), "r--")
    plt.xlabel('Time')
    plt.ylabel('OstQ')
    plt.text(0,0,"y=%.6fx+(%.6f)"%(z[0],z[1]))
    #plt.savefig('OstQ.png')
    plt.show()

