# interface

import os
import serial
from abc import *
import struct
from cli_const import *

def hex_msb(hex_str):
    if (len(hex_str) > 2):
        if (hex_str[:2] == '0x'):
            hex_str = hex_str[2:]

    hex_str_msb = ''.join([hex_str[i-2:i] for i in range(len(hex_str), 0, -2)])  
    
    # 다시 0x 를 붙여 나는 16진수다! 를 알려준다. 그러고 그대로 반환
    #  bac4812f3b > 0xbac4812f3b
    return '0x' + hex_str_msb

def hex_lsb(hex_str):
    if (len(hex_str) > 2):
        if (hex_str[:2] == '0x'):
            hex_str = hex_str[2:]
            
    try:
        # 2자리씩잘라 역순으로 변환 후 합친다.
        # 3b2f81c4ba > ba c4 81 2f 3b > bac4812f3b
        hex_str_lsb = ''.join([hex_str[i-2:i] for i in range(len(hex_str), 0, -2)])  
      
        # 다시 0x 를 붙여 나는 16진수다! 를 알려준다. 그러고 그대로 반환
        #  bac4812f3b > 0xbac4812f3b
        return '0x' + hex_str_lsb

    except ValueError:
        # 에러 처리!
        print('Conversion failed!')
        return ''

class DeviceIf(metaclass=ABCMeta):
    @abstractmethod
    def check(self):
        pass
    
    @abstractmethod
    def read(self, addr):
        pass

    @abstractmethod
    def write(self, addr, value):
        pass

class Uart(DeviceIf):
    def __init__(self, port, baudrate):
        self.port = port
        self.baudrate = baudrate

    def check(self):
        return os.path.exists(self.port)

    def read(self, addr):        
        ser = serial.Serial(
            port = self.port,
            baudrate = self.baudrate
        )

        #print(hex(addr))
        ser.write( ("GETREG " + hex(addr) + '\r').encode('UTF-8') )
        ser.flush()

        buffer = ser.readline().decode()

        ser.close()
        
        # split_list[1]
        # print(buffer)
        value = buffer.split(':')

        return '0x' + ('0' * (16 - len(value[1][3:-1]))) + value[1][3:-1]

    def write(self, addr, data):
        ser = serial.Serial(
            port = self.port,
            baudrate = self.baudrate
        )

        #print(hex(addr))
        ser.write( ("SETREG " + hex(addr) + ' ' + hex(data)  + '\r').encode('UTF-8') )
        ser.flush()

        ser.readline()

        ser.close()

class Pci(DeviceIf):
    def __init__(self, readFilename, writeFilename):
        self.readFilename = readFilename
        self.writeFilename = writeFilename

    def check(self):
        return os.path.exists(self.readFilename)

    def read(self, addr):        
        fd = os.open(self.readFilename, os.O_RDONLY)

        buffer = os.pread(fd, 8, addr)

        os.close(fd)

        return hex_lsb(buffer.hex())

    def write(self, addr, data):
        fd = os.open(self.writeFilename, os.O_RDWR)
     
        os.pwrite(fd, struct.pack('<Q', data), addr)

        os.close(fd)
        

class DeviceIfList():
    def __init__(self):
        PCI_CBUS_READ_FILE_NAME = '/dev/xdma0_c2h_0'
        PCI_CBUS_WRITE_FILE_NAME = '/dev/xdma0_h2c_0'
        PCI_DBUS_READ_FILE_NAME = '/dev/xdma0_c2h_1'
        PCI_DBUS_WRITE_FILE_NAME = '/dev/xdma0_h2c_1'

        UART_FILE_NAME = '/dev/ttyUSB2'
        UART_BAUD_RATE = 115200

        self.pciifCbus = Pci(PCI_CBUS_READ_FILE_NAME, PCI_CBUS_WRITE_FILE_NAME)
        self.pciifDbus = Pci(PCI_DBUS_READ_FILE_NAME, PCI_DBUS_WRITE_FILE_NAME)

        if (self.pciifCbus.check() == False):
            self.pciifCbus = None
            
        if (self.pciifDbus.check() == False):
            self.pciifDbus = None
        
        self.uartif = Uart(UART_FILE_NAME, UART_BAUD_RATE)
        if (self.uartif.check() == False):
            self.uartif = None


        self.numMuPerCluster = None
        self.ddrConfig = None        
        self.clstBmpPerSub = []
        self.readFpgaConfig()
        
    def setTarget(self, type, mts_core_id, subId, clusterId, muId, threadId):

        if (type == 'a'):
            assert(muId < MAX_ADMIN_MU_COUNT)
            subId = 0
            clusterId = 0 
            threadId = 0
            
        elif (type == 'm'):
            assert(clusterId < MAX_CLUSTER_PER_SUB_COUNT)
            assert(subId < MAX_SUB_COUNT)
            assert(muId == 0)
            assert(threadId == 0)
            clstEnabled = ((1 << clusterId) & self.clstBmpPerSub[subId]) > 0
            assert(clstEnabled == 1)
        else:
            assert(clusterId < MAX_CLUSTER_PER_SUB_COUNT)
            assert(subId < MAX_SUB_COUNT)
            assert(threadId < MAX_THREAD_COUNT)
            assert(muId < self.numMuPerCluster)
            clstEnabled = ((1 << clusterId) & self.clstBmpPerSub[subId]) > 0
            assert(clstEnabled == 1)

        self.type = type
        self.sub_id = subId
        self.cluster_id = clusterId
        self.mu_id = muId
        self.thread_id = threadId
        self.mts_core_id = mts_core_id

    def read(self, addr):        
        if (IsUartOnly(addr)):
            return self.uartif.read(addr)

        if (addr < DDR_BASE_ADDR):
            if (self.pciifCbus != None):
                return self.pciifCbus.read(addr)
            else:
                return self.uartif.read(addr)
        else:
            if (self.pciifDbus != None):
                return self.pciifDbus.read(addr)
            else:
                return self.uartif.read(addr)
            


    def write(self, addr, data):
        if (IsUartOnly(addr)):
            self.uartif.write(addr, data)
            return
        if (addr < DDR_BASE_ADDR):
            if (self.pciifCbus != None):
                return self.pciifCbus.write(addr, data)
            else:
                return self.uartif.write(addr, data)
        else:
            if (self.pciifDbus != None):
                return self.pciifDbus.write(addr, data)
            else:
                return self.uartif.write(addr, data)
            
    def readFpgaConfig(self):        
        DEVICE_CONFIG_ADDR = 0x00_C108_0000
        fpgaConfigQw0 = int(self.read(DEVICE_CONFIG_ADDR + 0x0), 0)
        fpgaConfigQw1 = int(self.read(DEVICE_CONFIG_ADDR + 0x8), 0)
        fpgaConfigQw2  = int(self.read(DEVICE_CONFIG_ADDR + 0x10) , 0)
        fpgaConfigQw3  = int(self.read(DEVICE_CONFIG_ADDR + 0x18) , 0)
        
        self.svnRevision = fpgaConfigQw0 
        self.ddrConfig = fpgaConfigQw2 & 0x1
        self.numMtsCore = ((fpgaConfigQw2 >> 4) & 0xF)
        self.adminPerSub = ((fpgaConfigQw2 >> 20) & 0xF)
        self.numMuPerCluster = (fpgaConfigQw3 & 0xFF)
        self.mufreq = (fpgaConfigQw3 >> 8) & 0xFF        
        self.dpefreq = (fpgaConfigQw3 >> 16) & 0xFF
        
        if self.numMtsCore == 0:
            self.numMtsCore = 1
        elif self.numMtsCore == 1:
            self.numMtsCore = 2

        clusterBitmap = fpgaConfigQw1 & 0xFFFF_FFFF
        for i in range(MAX_SUB_COUNT):
            self.clstBmpPerSub.append(clusterBitmap & 0xFF)
            clusterBitmap >>= 8
            

    def print_fpga_config(self):
        print("SVN Revision: " + str(self.svnRevision))
        print("DDR Config: " + str(self.ddrConfig))
        print("Number of Metis Core: " + str(self.numMtsCore))
        print("Number of MU per cluster: " + str(self.numMuPerCluster))
        print("mufreq: " + str(self.mufreq))
        print("dpefreq: " + str(self.dpefreq))
        for i in range(MAX_SUB_COUNT):
            print("Cluster bitmap for sub " + str(i) + ": " + hex(self.clstBmpPerSub[i]))

    def getClusterBmp(self, subId):
        return self.clstBmpPerSub[subId]
    
    def isValidCluster(self, subId, clusterId):
        return (self.clstBmpPerSub[subId] >> clusterId) & 0x1

    def cluster_generator(self):
        for subId in range(0, MAX_SUB_COUNT):
            for clusterId in range(0, MAX_CLUSTER_PER_SUB_COUNT):
                if (self.isValidCluster(subId, clusterId) == 1):
                    yield (subId, clusterId)
                
    def get_cluster_list(self):
        return list(self.cluster_generator())
    
    def isValidAdmin(self, muId):

        if self.adminPerSub == 0:
            subId = muId >> 1
            adminId = muId & 0x1
            if (subId >= MAX_SUB_COUNT):
                return 0
            clusterBmp = self.clstBmpPerSub[subId]
            numBitShift = 8 * adminId
            isValid = (clusterBmp >> numBitShift)
        else:
            if (MAX_SUB_COUNT > (muId * self.adminPerSub)) :
                isValid = True
            else:
                isValid = False

        return isValid
    
    def getNumMuPerCluster(self):
        return self.numMuPerCluster
    
    def getNumMtsCore(self):
        return self.numMtsCore
