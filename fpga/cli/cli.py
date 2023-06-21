#!/usr/bin/env python3

import deviceif
import click
from cli_const import *
from cli_functions import *
from gdbserver import *



# ==============================================================================
# Interface
# ==============================================================================


# ==============================================================================
# Click Main
# ==============================================================================

class NaturalOrderGroup(click.Group):
    def list_commands(self, ctx):
        return self.commands.keys()

@click.group(cls=NaturalOrderGroup)
@click.pass_context
def main(device):
    ''' MetisX Debugging Helper '''
    pass

# ==============================================================================
# CSR Functions
# ==============================================================================

@main.command(short_help='<mts_core_id> <sub_id> <cluster_id> <mu_id>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('cluster_id', type=click.INT)
@click.argument('mu_id', type=click.INT)
@click.pass_obj
def csr_slave(device, mts_core_id, sub_id, cluster_id, mu_id):
    device.setTarget('s', mts_core_id, sub_id, cluster_id, mu_id, 0)

    startAddr = getMuCsrAddr('s', mts_core_id, sub_id, cluster_id, mu_id)
    print(f"MU {mts_core_id} {sub_id} {cluster_id} {mu_id} {startAddr:x}")
    mu_csr(device, startAddr)

@main.command(short_help='<mts_core_id> <sub_id> <cluster_id>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('cluster_id', type=click.INT)
@click.pass_obj
def csr_master(device, mts_core_id, sub_id, cluster_id):
    device.setTarget('m', mts_core_id, sub_id, cluster_id, 0, 0)

    startAddr = getMuCsrAddr('m', mts_core_id, sub_id, cluster_id, 0)
    print(f"Master_MS {mts_core_id} {sub_id} {cluster_id} {startAddr:x}")
    mu_csr(device, startAddr)

@main.command(short_help='<mts_core_id> <mu_id>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('mu_id', type=click.INT)
@click.pass_obj
def csr_admin(device, mts_core_id, mu_id):
    device.setTarget('a',mts_core_id,  0, 0, mu_id, 0)

    startAddr = getMuCsrAddr('a', mts_core_id, 0, 0, mu_id)
    print(f"Admin_MS {mts_core_id} {mu_id} {startAddr:x}")
    mu_csr(device, startAddr)

@main.command(short_help='<mts_core_id> <sub_id> <cluster_id>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('cluster_id', type=click.INT)
@click.pass_obj
def csr_l1c(device, mts_core_id, sub_id, cluster_id):
    device.setTarget('m', mts_core_id, sub_id, cluster_id, 0, 0)

    startAddr = getL1cAddr(mts_core_id, sub_id, cluster_id)
    print(f"csr_l1c {mts_core_id} {sub_id} {cluster_id} {startAddr:x}")

    l1c_csr(device, startAddr)

@main.command(short_help='<mts_core_id> <l2c_id>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('l2c_id', type=click.INT)
@click.pass_obj
def csr_l2c(device, mts_core_id, l2c_id):
    device.setTarget('m', mts_core_id, 0, 0, 0, 0)

    startAddr = getL2cAddr(mts_core_id, l2c_id)
    print(f"csr_l2c {mts_core_id} {l2c_id} {startAddr:x}")

    l2c_csr(device, startAddr)



# ==============================================================================
# Stat Functions
# ==============================================================================

@main.command(short_help='<mts_core_id> <sub_id> <cluster_id>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('cluster_id', type=click.INT)
@click.pass_obj
def stat_master(device, mts_core_id, sub_id, cluster_id):
    device.setTarget('m', mts_core_id, sub_id, cluster_id, 0, 0)

    startAddr = getStatAddr('m', mts_core_id, sub_id, cluster_id, 0)
    print(f"mastermu_stat {mts_core_id} {sub_id} {cluster_id} {startAddr:x}")
    mu_stat(device, startAddr)

@main.command(short_help='<mts_core_id> <mu_id>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('mu_id', type=click.INT)
@click.pass_obj
def stat_admin(device, mts_core_id, mu_id):
    device.setTarget('a', mts_core_id, 0, 0, mu_id, 0)

    startAddr = getStatAddr('a', mts_core_id, 0, 0, mu_id)
    print(f"adminmu_stat {mts_core_id} {mu_id} {startAddr:x}")
    mu_stat(device, startAddr)

# ==============================================================================
# GDB Functions
# ==============================================================================

@main.command(short_help='<mts_core_id> <sub_id> <cluster_id> <mu_id> <thread_id> <port>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('cluster_id', type=click.INT)
@click.argument('mu_id', type=click.INT)
@click.argument('thread_id', type=click.INT)
@click.argument('port', type=click.INT)
@click.pass_obj
def gdb_slave(device, mts_core_id, sub_id, cluster_id, mu_id, thread_id, port):
    device.setTarget('s', mts_core_id,  sub_id, cluster_id, mu_id, thread_id)

    try:
        gdbclient = GDBClientHandler(device, 's', sub_id, cluster_id, mu_id, thread_id, port)
        gdbclient.run()
    except KeyboardInterrupt:
        gdbclient.close()

@main.command(short_help='<mts_core_id> <sub_id> <cluster_id> <port>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('cluster_id', type=click.INT)
@click.argument('port', type=click.INT)
@click.pass_obj
def gdb_master(device, mts_core_id, sub_id, cluster_id, port):
    device.setTarget('m', mts_core_id, sub_id, cluster_id, 0, 0)

    try:
        gdbclient = GDBClientHandler(device, 'm', sub_id, cluster_id, 0, 0, port)
        gdbclient.run()
    except KeyboardInterrupt:
        gdbclient.close()

@main.command(short_help='<mts_core_id> <mu_id> <port>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('mu_id', type=click.INT)
@click.argument('port', type=click.INT)
@click.pass_obj
def gdb_admin(device, mts_core_id, mu_id, port):
    device.setTarget('a', mts_core_id, 0, mu_id, 0)
    try:
        gdbclient = GDBClientHandler(device, 'a', 0, mu_id, 0, port)
        gdbclient.run()
    except KeyboardInterrupt:
        gdbclient.close()

# ==============================================================================
# Stack Functions
# ==============================================================================

@main.command(short_help='<mts_core_id> <sub_id> <cluster_id> <mu_id> <thread_id> <offset> [range=10]')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('cluster_id', type=click.INT)
@click.argument('mu_id', type=click.INT)
@click.argument('thread_id', type=click.INT)
@click.argument('offset', type=click.INT)
@click.argument('range', default=10, type=click.INT)
@click.pass_obj
def sp_slave(device, mts_core_id, sub_id, cluster_id, mu_id, thread_id, offset, range):
    device.setTarget('s', mts_core_id, sub_id, cluster_id, mu_id, thread_id)

    reg_addr = getRegisterAddr('s', mts_core_id, sub_id, cluster_id, mu_id, thread_id, 0)
    stack_addr = getStackAddr('s', mts_core_id, sub_id, cluster_id, mu_id, thread_id)
    mu_stack(device, stack_addr, reg_addr, offset, range)

@main.command(short_help='<mts_core_id> <sub_id> <cluster_id> <offset> [range=10]')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('cluster_id', type=click.INT)
@click.argument('offset', type=click.INT)
@click.argument('range', default=10, type=click.INT)
@click.pass_obj
def sp_master(device, mts_core_id, sub_id, cluster_id, offset, range):
    device.setTarget('m', mts_core_id, sub_id, cluster_id, 0, 0)

    reg_addr = getRegisterAddr('m', mts_core_id, sub_id, cluster_id, 0, 0, 0)
    stack_addr = getStackAddr('m', mts_core_id, sub_id, cluster_id, 0, 0)
    mu_stack(device, stack_addr, reg_addr, offset, range)

@main.command(short_help='<mts_core_id> <mu_id> <offset> [range=10]')
@click.argument('mts_core_id', type=click.INT)
@click.argument('mu_id', type=click.INT)
@click.argument('offset', type=click.INT)
@click.argument('range', default=10, type=click.INT)
@click.pass_obj
def sp_admin(device, mts_core_id, mu_id, offset, range):
    device.setTarget('a', mts_core_id,  0, 0, mu_id, 0)

    reg_addr = getRegisterAddr('a', mts_core_id, 0, 0, mu_id, 0, 0)
    stack_addr = getStackAddr('a', mts_core_id, 0, 0, mu_id, 0)
    mu_stack(device, stack_addr, reg_addr, offset, range)


# ==============================================================================
# map Functions
# ==============================================================================

@main.command(short_help='<mts_core_id> <sub_id> <cluster_id> <mu_id> <thread_id>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('cluster_id', type=click.INT)
@click.argument('mu_id', type=click.INT)
@click.argument('thread_id', type=click.INT)
@click.pass_obj
def map_slave(device, mts_core_id, sub_id, cluster_id, mu_id, thread_id):
    device.setTarget('s', mts_core_id, sub_id, cluster_id, mu_id, thread_id)
    mu_memory_map(device, 's', mts_core_id, sub_id, cluster_id, mu_id, thread_id )

@main.command(short_help='<mts_core_id> <sub_id> <cluster_id>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('sub_id', type=click.INT)
@click.argument('cluster_id', type=click.INT)
@click.pass_obj
def map_master(device, mts_core_id, sub_id, cluster_id):
    device.setTarget('m', mts_core_id, sub_id, cluster_id, 0, 0)
    mu_memory_map(device, 'm', mts_core_id, sub_id, cluster_id, 0, 0 )

@main.command(short_help='<mts_core_id> <mu_id>')
@click.argument('mts_core_id', type=click.INT)
@click.argument('mu_id', type=click.INT)
@click.pass_obj
def map_admin(device, mts_core_id, mu_id):
    device.setTarget('a', mts_core_id, 0, 0, mu_id, 0)
    mu_memory_map(device, 'a', mts_core_id, 0, 0, mu_id, 0 )

# ==============================================================================
# Fpga Config
# ==============================================================================

# get Fpga Config
@main.command()
@click.pass_obj
def fpga_config(device):
    device.setTarget('a', 0, 0, 0, 0, 0)
    device.print_fpga_config()

# ==============================================================================
# MBox Functions
# ==============================================================================

@main.command(short_help='[-skip_empty]')
@click.option('-skip_empty', is_flag=True, default=False)
@click.pass_obj
def hmbox(device, skip_empty):
    numMtsCore   = device.getNumMtsCore()
    for mtsCoreId in range(numMtsCore):
        print(f"=============== MTS_CORE {mtsCoreId} ===============")
        mbox_print_hmbox_wq(device, mtsCoreId, skip_empty)
        mbox_print_hmbox_rq(device, mtsCoreId, skip_empty)

@main.command(short_help='[-skip_empty]')
@click.option('-skip_empty', is_flag=True, default=False)
@click.pass_obj
def adm_mbox(device, skip_empty):
    
    numMtsCore   = device.getNumMtsCore()
    for mtsCoreId in range(numMtsCore):
        print(f"=============== MTS_CORE {mtsCoreId} ===============")
        mbox_print_adm_mbox_wq(device, mtsCoreId, skip_empty)
        mbox_print_adm_mbox_rq(device, mtsCoreId, skip_empty)

@main.command(short_help='[-skip_empty]')
@click.option('-skip_empty', is_flag=True, default=False)
@click.pass_obj
def mts_mbox(device, skip_empty):
    numMtsCore   = device.getNumMtsCore()
    for mtsCoreId in range(numMtsCore):
        print(f"=============== MTS_CORE {mtsCoreId} ===============")
        mbox_print_mts_mbox_wq(device, mtsCoreId, skip_empty)
        mbox_print_mts_mbox_rq(device, mtsCoreId, skip_empty)

# ==============================================================================
# Debug Functions
# ==============================================================================
@main.command()
@click.pass_obj
def debug_pc(device):
    mu_print_debug_pc(device)

@main.command(short_help='[offset]')
@click.option('--offset', type=click.INT, required=False, default=0, help="PC offset")
@click.pass_obj
def debug_inst(device, offset):
    mu_print_debug_pc(device, offset, True)

@main.command(short_help='[-skip_empty]')
@click.pass_obj
@click.option('-skip_empty', is_flag=True, default=False)
def debug_mbox(device, skip_empty):
    numMtsCore   = device.getNumMtsCore()
    for mtsCoreId in range(numMtsCore):
        print(f"=============== MTS_CORE {mtsCoreId} ===============")
        mbox_print_ctx_all(device, mtsCoreId, skip_empty)

@main.command(short_help='exclude e.g, grep -v mu_error')
@click.option('-l1c', is_flag=True, default=True)
@click.option('-l2c', is_flag=True, default=True)
@click.pass_obj
def debug_error(device, l1c, l2c):
    allmu(device, mu_error)

    if (l1c):
        allL1Cache(device, cache_error)

    if (l2c):
        allL2Cache(device, cache_error)
    mbox_print_dbg(device)


@main.command()
@click.pass_obj
def debug_stat(device):
    allmu(device, mu_stat, getStatAddr)

@main.command(short_help='<start_addr(hex)> <end_addr(hex) or size(hex)<=0x2000 > [length=2] [-l]')
@click.argument('start_addr', type=click.STRING)
@click.argument('end_addr', type=click.STRING)
@click.argument('length', default=2, type=click.INT)
@click.option('-l', is_flag=True, default=False)
@click.pass_obj
def dump(device, start_addr, end_addr, length, l):
    start_addr = start_addr.replace('_', '')
    end_addr = end_addr.replace('_', '')

    if (int(end_addr, 16) <= 0x2000):
        startAddrInt = int(start_addr, 16)
        checkAddr(startAddrInt)
        endAddrInt = startAddrInt + int(end_addr, 16)
        checkAddr(endAddrInt)
    else:
        startAddrInt = int(start_addr, 16)
        checkAddr(startAddrInt)
        endAddrInt = int(end_addr, 16)
        checkAddr(endAddrInt)


    if (length == 1):
        def AddPrintFunc(addr, hexValue):
            #s2 = "".join([chr(i) if 32 <= i <= 127 else "." for i in bytes(int(hexValue, 16)) ]) # ascii string; chained comparison
            s2 = "".join([chr(i) if 32 <= i <= 127 else "." for i in reversed(struct.pack('<Q', int(hexValue, 16))) ] )
            print(s2 , end='')

        hexdump(device, startAddrInt, endAddrInt, length=length, littleEndian=l, addPrintFunc=AddPrintFunc)
    else:
        hexdump(device, startAddrInt, endAddrInt, length=length, littleEndian=l)


@main.command(short_help='<addr(hex)> <value(hex)>')
@click.argument('addr', type=click.STRING)
@click.argument('value', type=click.STRING)
@click.pass_obj
def writeqw(device, addr, value):
    addr = addr.replace('_', '')
    value = value.replace('_', '')

    addrInt = int(addr, 16)
    checkAddr(addrInt)

    device.write(addrInt, int(value, 16) )

@main.command(short_help='<cluster_id>')
@click.argument('cluster_id', type=click.INT)
@click.pass_obj
def busmon(device, cluster_id):
    device.setTarget('s', 0, cluster_id, 0, 0)
    bus_monitor(device)

if __name__ == "__main__":
	main(obj=deviceif.DeviceIfList())
