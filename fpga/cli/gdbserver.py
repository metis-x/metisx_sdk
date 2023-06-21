
import sys
import socket
import logging
import struct
from urllib.response import addinfo
from cli_const import *
from cli_functions import *
from deviceif import *

GDB_SIGNAL_TRAP = 5

def checksum(data):
    checksum = 0
    for c in data:
        checksum += ord(c)
    return checksum & 0xff

def ReadMemory(addr, size):
    return '0' * size

def StepInfo():
    pass

def GetCpuThreadId():
    return 0

# Code a bit inspired from http://mspgcc.cvs.sourceforge.net/viewvc/mspgcc/msp430simu/gdbserver.py?revision=1.3&content-type=text%2Fplain
class GDBClientHandler(object):
    def __init__(self, device, type, subId, clusterId, muId, threadId, port):
        logging.basicConfig(level = logging.WARN)
        for logger in 'gdbclienthandler runner main'.split(' '):
            logging.getLogger(logger).setLevel(level = logging.DEBUG)

        log = logging.getLogger('main')
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(('', port))
        log.info('listening on :%d' % port)
        sock.listen(1)
        clientsocket, addr = sock.accept()
        log.info('connected')

        self.device = device
        self.subId = subId
        self.type = type
        self.clusterId = clusterId
        self.muId = muId
        self.threadId = threadId

        self.clientsocket = clientsocket
        self.netin = clientsocket.makefile('r')
        self.netout = clientsocket.makefile('w')
        self.log = logging.getLogger('gdbclienthandler')
        self.last_pkt = None

    def close(self):
        '''End of story!'''
        self.netin.close()
        self.netout.close()
        self.clientsocket.close()
        self.log.info('closed')

    def read(self, addr):
        return hex_msb(self.device.read(addr))

    def run(self):
        '''Some doc about the available commands here:
            * http://www.embecosm.com/appnotes/ean4/embecosm-howto-rsp-server-ean4-issue-2.html#id3081722
            * http://git.qemu.org/?p=qemu.git;a=blob_plain;f=gdbstub.c;h=2b7f22b2d2b8c70af89954294fa069ebf23a5c54;hb=HEAD +
             http://git.qemu.org/?p=qemu.git;a=blob_plain;f=target-i386/gdbstub.c;hb=HEAD'''
        self.log.info('client loop ready...')

        while self.receive() == 'Good':
            pkt = self.last_pkt
            self.log.debug('receive(%r)' % pkt)
            # Each packet should be acknowledged with a single character. '+' to indicate satisfactory receipt
            self.send_raw('+')

            def handle_q(subcmd):
                '''
                subcmd Supported: https://sourceware.org/gdb/onlinedocs/gdb/General-Query-Packets.html#qSupported
                Report the features supported by the RSP server. As a minimum, just the packet size can be reported.
                '''
                if subcmd.startswith('Supported'):
                    self.log.info('Received qSupported command')
                    self.send('PacketSize=%x' % 4096)
                elif subcmd.startswith('Attached'):
                    self.log.info('Received qAttached command')
                    # https://sourceware.org/gdb/onlinedocs/gdb/General-Query-Packets.html
                    self.send('0')
                elif subcmd.startswith('C'):
                    self.send('T%.2x;' % GetCpuThreadId())
                elif subcmd.startswith('RegisterInfo'):
                    index = int(subcmd[len('RegisterInfo'):], 16)

                    registerInfo = [
                        "name:zero;alt-name:x0;bitsize:64;offset:0;encoding:uint;format:hex;set:GPR;ehframe:0;dwarf:0;",
                        "name:sp;alt-name:x1;bitsize:64;offset:8;encoding:uint;format:hex;set:GPR;ehframe:1;dwarf:1;generic:sp;",
                        "name:ra;alt-name:x2;bitsize:64;offset:16;encoding:uint;format:hex;set:GPR;ehframe:2;dwarf:2;generic:ra;",
                        "name:s0;alt-name:x3;bitsize:64;offset:24;encoding:uint;format:hex;set:GPR;ehframe:3;dwarf:3;",
                        "name:s1;alt-name:x4;bitsize:64;offset:32;encoding:uint;format:hex;set:GPR;ehframe:4;dwarf:4;",
                        "name:s2;alt-name:x5;bitsize:64;offset:40;encoding:uint;format:hex;set:GPR;ehframe:5;dwarf:5;",
                        "name:t0;alt-name:x6;bitsize:64;offset:48;encoding:uint;format:hex;set:GPR;ehframe:6;dwarf:6;",
                        "name:t1;alt-name:x7;bitsize:64;offset:56;encoding:uint;format:hex;set:GPR;ehframe:7;dwarf:7;",
                        "name:t2;alt-name:x8;bitsize:64;offset:64;encoding:uint;format:hex;set:GPR;ehframe:8;dwarf:8;",
                        "name:t3;alt-name:x9;bitsize:64;offset:72;encoding:uint;format:hex;set:GPR;ehframe:9;dwarf:9;",
                        "name:t4;alt-name:x10;bitsize:64;offset:80;encoding:uint;format:hex;set:GPR;ehframe:10;dwarf:10;",
                        "name:t5;alt-name:x11;bitsize:64;offset:88;encoding:uint;format:hex;set:GPR;ehframe:11;dwarf:11;",
                        "name:a0;alt-name:x12;bitsize:64;offset:96;encoding:uint;format:hex;set:GPR;ehframe:12;dwarf:12;",
                        "name:a1;alt-name:x13;bitsize:64;offset:104;encoding:uint;format:hex;set:GPR;ehframe:13;dwarf:13;generic:arg1;",
                        "name:a2;alt-name:x14;bitsize:64;offset:112;encoding:uint;format:hex;set:GPR;ehframe:14;dwarf:14;generic:arg2;",
                        "name:a3;alt-name:x15;bitsize:64;offset:120;encoding:uint;format:hex;set:GPR;ehframe:15;dwarf:15;generic:arg3;",
						"name:a4;alt-name:x16;bitsize:64;offset:128;encoding:uint;format:hex;set:GPR;ehframe:16;dwarf:16;generic:arg4;",
						"name:a5;alt-name:x17;bitsize:64;offset:136;encoding:uint;format:hex;set:GPR;ehframe:17;dwarf:17;generic:arg5;",
						"name:a6;alt-name:x18;bitsize:64;offset:144;encoding:uint;format:hex;set:GPR;ehframe:18;dwarf:18;generic:arg6;",
						"name:a7;alt-name:x19;bitsize:64;offset:152;encoding:uint;format:hex;set:GPR;ehframe:19;dwarf:19;generic:arg7;",
						"name:s3;alt-name:x20;bitsize:64;offset:160;encoding:uint;format:hex;set:GPR;ehframe:20;dwarf:20;",
						"name:s4;alt-name:x21;bitsize:64;offset:168;encoding:uint;format:hex;set:GPR;ehframe:21;dwarf:21;",
						"name:s5;alt-name:x22;bitsize:64;offset:176;encoding:uint;format:hex;set:GPR;ehframe:22;dwarf:22;",
						"name:s6;alt-name:x23;bitsize:64;offset:184;encoding:uint;format:hex;set:GPR;ehframe:23;dwarf:23;",
						"name:s7;alt-name:x24;bitsize:64;offset:192;encoding:uint;format:hex;set:GPR;ehframe:24;dwarf:24;",
						"name:s8;alt-name:x25;bitsize:64;offset:200;encoding:uint;format:hex;set:GPR;ehframe:25;dwarf:25;",
						"name:s9;alt-name:x26;bitsize:64;offset:208;encoding:uint;format:hex;set:GPR;ehframe:26;dwarf:26;",
						"name:s10;alt-name:x27;bitsize:64;offset:216;encoding:uint;format:hex;set:GPR;ehframe:27;dwarf:27;",
						"name:s11;alt-name:x28;bitsize:64;offset:224;encoding:uint;format:hex;set:GPR;ehframe:28;dwarf:28;",
						"name:s12;alt-name:x29;bitsize:64;offset:232;encoding:uint;format:hex;set:GPR;ehframe:29;dwarf:29;",
						"name:t6;alt-name:x30;bitsize:64;offset:240;encoding:uint;format:hex;set:GPR;ehframe:30;dwarf:30;",
						"name:t7;alt-name:x31;bitsize:64;offset:248;encoding:uint;format:hex;set:GPR;ehframe:31;dwarf:31;",
                        "name:pc;bitsize:64;offset:256;encoding:uint;format:hex;set:SR;ehframe:32;dwarf:32;generic:pc;",
                    ]

                    if (index < len(registerInfo)):                    
                        self.send(registerInfo[index])
                    else:
                        self.send('OK')
                else:
                    self.log.error('This subcommand %r is not implemented in q' % subcmd)
                    self.send('')

            def handle_h(subcmd):
                self.send('OK')

            def handle_qmark(subcmd):
                self.send('S%.2x' % GDB_SIGNAL_TRAP)

            def handle_g(subcmd):
                if subcmd == '':
                    # EAX, ECX, EDX, ESP, EBP, ESI, EDI, EIP, EFLAGS, CS, SS, DS, ES, FS, GS
                    registers = [ 
                        self.read( getRegisterAddr(self.type, self.subId, self.clusterId, self.muId, self.threadId, x) )[2:] for x in range(32) 
                    ]
                    registers.append( self.read(getPcAddr(self.type, self.subId, self.clusterId, self.muId, self.threadId))[2:] )

                    print(registers)
                    s = ''.join(registers)
                    #for r in registers:
                    #    s += struct.pack(

                    print(s)
                    self.send(s)

            def ReadMemory(addr, size):
                addr = addr & ((1 << 40) - 1)
                def remapText(startAddr, addr):
                    offset = addr - startAddr
                    return getTextAddr(self.device, self.type, self.subId, self.clusterId, self.muId, self.threadId) + offset
                
                def remapStack(startAddr, addr):
                    offset = addr - startAddr
                    return getStackAddr(self.type, self.subId, self.clusterId, self.muId, self.threadId) + offset
                   
                def remap4M(startAddr, addr):
                    offset = addr - startAddr                    
                    return getRemap4M(self.device, self.type, self.subId, self.clusterId, self.muId, self.threadId) + offset

                if (addr > 0xFF_FFFF_FFFF):                
                    return 'E0'

                upSize = int( (size + 7) / 8) * 8

                remapInfos = [
                    {'start' : 0x0000_0000, 'end' : 0x0001_0000, 'remapfunc' : remapText},
                    {'start' : 0x1000_0000, 'end' : 0x2000_0000, 'remapfunc' : remap4M},
                    {'start' : 0x3000_0000, 'end' : 0x4000_0000, 'remapfunc' : remapStack},
                ]

                remapAddr = addr
                for remapInfo in remapInfos:
                    if (remapInfo['start'] <= addr and addr <= remapInfo['end']):
                        remapAddr = remapInfo['remapfunc'](remapInfo['start'], addr)
                        # print("%0x" % remapAddr, remapInfo['remapfunc'].__name__)                        
                        break

                s = ''
                for readAddr in range(remapAddr, remapAddr + upSize, 8):
                    s += self.read(readAddr)[2:]

                return s

            def handle_m(subcmd):
                addr, size = subcmd.split(',')
                addr = int(addr, 16)
                size = int(size, 16)
                self.log.info('Received a "read memory" command (@%#.8x : %d bytes)' % (addr, size))
                self.send(ReadMemory(addr, size))

            def handle_s(subcmd):    
                self.log.info('Received a "single step" command')
                self.send('T%.2x' % GDB_SIGNAL_TRAP)

            dispatchers = {
                'q' : handle_q,
                'H' : handle_h,
                '?' : handle_qmark,
                'g' : handle_g,
                'm' : handle_m,
                's' : handle_s,
                'x' : handle_m
            }

            cmd, subcmd = pkt[0], pkt[1 :]
            if cmd == 'k':
                break

            if cmd not in dispatchers:
                self.log.info('%r command not handled' % pkt)
                self.send('')
                continue

            dispatchers[cmd](subcmd)

        self.close()

    def receive(self):
        '''Receive a packet from a GDB client'''
        # XXX: handle the escaping stuff '}' & (n^0x20)
        csum = 0
        state = 'Finding SOP'
        packet = ''
        while True:
            c = self.netin.read(1)
            if c == '\x03':
                return 'Error: CTRL+C'
            
            if len(c) != 1:
                return 'Error: EOF'

            if state == 'Finding SOP':
                if c == '$':
                    state = 'Finding EOP'
            elif state == 'Finding EOP':
                if c == '#':
                    if csum != int(self.netin.read(2), 16):
                        raise Exception('invalid checksum')
                    self.last_pkt = packet
                    return 'Good'
                else:
                    packet += c
                    csum = (csum + ord(c)) & 0xff               
            else:
                raise Exception('should not be here')

    def send(self, msg):
        '''Send a packet to the GDB client'''
        self.log.debug('send(%r)' % msg)
        self.send_raw('$%s#%.2x' % (msg, checksum(msg)))

    def send_raw(self, r):
        self.netout.write(r)
        self.netout.flush()     