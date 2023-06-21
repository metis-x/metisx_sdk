from cli_const import *
from deviceif import *
from Font import *
from disassembler import *
# ==============================================================================
# Help Functions
# ==============================================================================

def filterZero(s):
    if (int(s, 16) != 0):
        return Font.yellow(s)

    return s

def isValidBit(bitmap, bit):
    return (bitmap >> bit) & 0x1

def getTypeString(type, mtsCoreId, subId, clusterId, muId, threadId):
    if (type == 'a'):
        return f'ADMIN[{mtsCoreId}][{subId}][{muId}]'
    elif (type == 's'):
        return f'SLAVE[{mtsCoreId}][{subId}][{clusterId}:{muId}:{threadId}]'
    elif (type == 'm'):
        return f'MASTER[{mtsCoreId}][{subId}][{clusterId}]'

    assert(0)


def readableAddr(s, splitCount = 4, filterFunc = filterZero):
    hexString = s
    if (len(s) > 2):
        if(s[:2] == '0x'):
            hexString = s[2:]

    readableStr = '0x'
    if ((len(hexString) % splitCount) != 0):
        readableStr += filterFunc(hexString[:len(hexString) % splitCount])
        for offset in range(len(hexString) % splitCount, len(hexString), splitCount):
            readableStr += '_' + filterFunc(hexString[offset:offset+splitCount])
    else:
        for offset in range(0, len(hexString), splitCount):
            readableStr += filterFunc(hexString[offset:offset+splitCount])
            if ( (offset + splitCount) < len(hexString)):
                readableStr += '_'

    return readableStr


def defaultFilter(value):
    return False

def PRT_ADDR(name, addr, valueStr):
    print("%s %-45s = %s" % (Font.bright_black(f"(0x{addr:010x})"), name, readableAddr(valueStr)) )

def PRT_BITS(name, addr, startbit, endbit, valueStr):
    value = int(valueStr, 16)

    endmask = (1 << (endbit + 1)) - 1
    startmask = (1 << startbit) - 1
    valueMask = endmask & ~startmask

    bitValue = (value & valueMask) >> startbit

    print("    %-56s = %s (%s)" % ((name) + '[' + str(endbit) + ':' + str(startbit) + ']', str(bitValue), hex(bitValue) ))

def PRT_INST(device, name, valueStr):
    INSTRUCTION_BYTE_SIZE = 4
    PRINT_INSTRUCTION_LINE = 5

    cur_pc = int(valueStr, 16)
    pc = int(valueStr, 16) - (INSTRUCTION_BYTE_SIZE * int(PRINT_INSTRUCTION_LINE / 2));
    if ('MU_CSR_REG_FETCH_PC' == name):
        for offset in range(0, (INSTRUCTION_BYTE_SIZE * PRINT_INSTRUCTION_LINE), INSTRUCTION_BYTE_SIZE):
            if (pc + offset >= 0x40):
                inst = mu_inst_from_device(device, pc + offset)
                if (pc + offset) == cur_pc:
                    print(Font.bright_red("    %-56s = " % ('[' + hex(pc + offset) + ']')), end='')
                else:
                    print("    %-56s = " % ('[' + hex(pc + offset) + ']'), end='')
                printDisassemble(inst)

def specialName(name, idx):
    if (name.startswith('MU_CSR_MON_GPR_THREAD')):
        registerName = ["zero","sp","ra","s0","s1","s2","t0","t1","t2","t3","t4","t5","a0","a1","a2","a3","a4","a5","a6","a7","s3","s4","s5","s6","s7","s8","s9","s10","s11","s12","t6","t7"]

        color_number = [Font.bright_magenta(str(0)), Font.bright_green(str(1)), Font.bright_red(str(2)), Font.bright_cyan(str(3))]

        number = name[len('MU_CSR_MON_GPR_THREAD')]

        name = 'MU_CSR_MON_GPR_THREAD' + color_number[int(number)] + name[len('MU_CSR_MON_GPR_THREAD')+1:]

        name += f"[{registerName[idx]}]"

    return name

def PRT(device, name, addr, arraySize, bitfields, filterFunc=defaultFilter):
    if (arraySize == 1):
        valueStr = device.read(addr)

        if (filterFunc(valueStr) == True):
            return

        if (valueStr.lower() == '0xdeaddeaddeaddead'):
            return

        PRT_ADDR(name, addr, valueStr)

        for bitfield in bitfields:
            PRT_BITS(bitfield[0], addr, bitfield[1], bitfield[2], valueStr)
    else:
        for idx in range(arraySize):
            arrName = name + f"_{idx:<3}"
            arrName = specialName(arrName, idx)

            arrAddr = addr + idx * 8
            valueStr = device.read(arrAddr)

            if (filterFunc(valueStr) == True):
                continue

            if (valueStr.lower() == '0xdeaddeaddeaddead'):
                continue

            PRT_ADDR(arrName, arrAddr, valueStr)

            PRT_INST(device, name, valueStr)

            for bitfield in bitfields:
                PRT_BITS(bitfield[0], arrAddr, bitfield[1], bitfield[2], valueStr)

def READ_PRT(device, name, addr):
    PRT_ADDR(name, addr + 0, device.addr(addr) )

def hexdump(device, startAddr, endAddr, **kwargs):
    loopCnt = 0
    length = 2
    littleEndian = False

    if 'littleEndian' in kwargs:
        littleEndian = kwargs['littleEndian']

    if 'length' in kwargs:
        length = kwargs['length']

    alignedStartAddr = int(startAddr / 8) * 8
    alginedEndAddr = int((endAddr + 7) / 8) * 8

    print("             %s" % ("=" * 22 * length) )
    if (littleEndian):
        print("             |%s" % (" L                 M |" * length) )
    else:
        print("             |%s" % (" M                 L |" * length) )
    print("             %s" % ("=" * 22 * length) )

    for addr in range(alignedStartAddr, alginedEndAddr, 8):
        if ((loopCnt % length) == 0):
            print(f'{Font.bright_black(f"0x{addr:010x}")} |' , end='')

        hexString = device.read(addr)

        if (littleEndian):
            hexString = hex_msb(hexString)

        print(readableAddr(hexString), end='|')

        if 'addPrintFunc' in kwargs:
            kwargs['addPrintFunc'](addr, hexString)

        loopCnt = loopCnt + 1
        if ((loopCnt % length) == 0):
            print()

    print()

def allmu(device, func, addrFunc = getMuCsrAddr):
    
    numMtsCore   = device.getNumMtsCore()
    for mtsCoreId in range(numMtsCore):
        print(f'===== MTS_CORE[{mtsCoreId}] {func.__name__} =====')
        # admin mu
        for muId in range(MAX_ADMIN_MU_COUNT):
            if(device.isValidAdmin(muId) == 1):
                addr = addrFunc('a', mtsCoreId, 0, 0, muId)
                if (addr == 0):
                    continue

                print(f'===== ADMIN[{muId}] {func.__name__} =====')

                func(device, addr)

        # master mu
        for (subId, clusterId) in device.get_cluster_list():
            addr = addrFunc('m', mtsCoreId, subId, clusterId, 0)
            if (addr == 0):
                continue
            print(f'===== MASTER[{subId}][{clusterId}] {func.__name__} =====')

            func(device, addr)


        # slave mu
        numMuPerCluster = device.getNumMuPerCluster()
        for (subId, clusterId) in device.get_cluster_list():
            for muId in range(numMuPerCluster):
                addr = addrFunc('s', mtsCoreId, subId, clusterId, muId)
                if (addr == 0):
                    continue

                print(f'===== SLAVE [{mtsCoreId}][{subId}][{clusterId}][{muId}] {func.__name__} =====')

                func(device, addr)


def allL1Cache(device, func):
    numMtsCore   = device.getNumMtsCore()
    for mtsCoreId in range(numMtsCore):
        for (subId, clusterId) in device.get_cluster_list():
            addr = getL1cAddr(mtsCoreId, subId, clusterId)
            print(f'===== L1C [{mtsCoreId}][{subId}][{clusterId}] {func.__name__} =====')
            func(device, addr)

def allL2Cache(device, func):
    numMtsCore   = device.getNumMtsCore()
    for mtsCoreId in range(numMtsCore):
        for index in range(4):
            addr = getL2cAddr(mtsCoreId, index)
            print(f'===== L2C [{mtsCoreId}][{index}] {func.__name__} =====')
            func(device, addr)


# ==============================================================================
# Custom Functions
# ==============================================================================

def mu_pc(device, startAddr):
    def filterFunc(valueStr):
        if (valueStr == '0x0000000000000080'):
            return True
        return False

    PRT(device, "MU_CSR_REG_MON_FETCH_PC"               , startAddr + 0x80, 4, [], filterFunc)

def mu_error(device, startAddr):
    def filterFunc(valueStr):
        if (valueStr == '0x0000000000000000'):
            return True
        return False

    PRT(device, "MU_CSR_W1C_MON_IRQ"                    , startAddr + 0x120, 1, [["IRQ_AXI_BRESP_ERROR", 0, 0], ["IRQ_AXI_RRESP_ERROR", 4, 4], ["IRQ_COP_DIV_ZERO", 8, 8], ["IRQ_COP_ERROR_TID", 12, 13], ["IRQ_SW_DEBUG_BREAK", 20, 20], ["IRQ_MU_ERROR", 24, 27], ["IRQ_EXT_ERROR", 28, 28], ["IRQ_RID_ITLV_ERROR", 32, 32], ["MPU_ERROR", 40, 47], ])


def mu_inst_from_device(device, pc):
    instAddr = getTextAddr(device, device.mts_core_id, device.type, device.sub_id, device.cluster_id, device.mu_id, device.thread_id) + pc

    return get4ByteData(device, instAddr)

def mu_inst_from_param(device, mtsCoreId, type, subId, clusterId, muId, threadId, pc):
    instAddr = getTextAddr(device, mtsCoreId,type, subId, clusterId, muId, threadId) + pc

    return get4ByteData(device, instAddr)

def mu_print_debug_pc(device, pcOffset=0, isInst=False):

    def get_list_offset(mtsCoreId, subId, clusterId, muId, threadId):
        return f'{mtsCoreId}_{subId}_{clusterId}_{muId}_{threadId}'

    numMuPerClst = device.getNumMuPerCluster()
    numMtsCore   = device.getNumMtsCore()

    pcList = {}
    slaveInst = {}
    sortedpcList = []
    for mtsCoreId in range(numMtsCore):
        for subId, clusterId in device.get_cluster_list():
            for muId in range(numMuPerClst):
                for threadId in range(MAX_THREAD_COUNT):
                    offset = get_list_offset(mtsCoreId, subId, clusterId, muId, threadId)
                    pcList[offset] = int(device.read(getPcAddr('s', mtsCoreId, subId, clusterId, muId, threadId)), 16)
                    if True == isInst:
                        slaveInst[offset] = getInstructionName(mu_inst_from_param(device, 's', mtsCoreId, subId, clusterId, muId, threadId, pcList[offset] + pcOffset))

    adminId = 0
    printPadding = "============"
    printPaddingHalf = "========"

    extraPrintPadding = ""

    for i in range(8, numMuPerClst):
        extraPrintPadding += printPadding

    for mtsCoreId in range(numMtsCore):
        mtsCorePrint = Font.magenta(f'[{mtsCoreId}]')
        for (subId, clusterId) in device.get_cluster_list():
            adminPrint = None
            adminPc = None
            if clusterId % 4 == 0:
                adminId = (subId << 1)
                adminStr = 'Admin' + str(adminId)
                adminPc = int(device.read(getPcAddr('a', mtsCoreId, 0, 0, adminId, 0)), 16)
                if True == isInst:
                    adminInst = f"{getInstructionName(mu_inst_from_param(device, 'a', mtsCoreId, 0, 0, adminId, 0, adminPc + pcOffset)):^7}"
                    adminPrint = f"{Font.red(adminStr +' '+ adminInst)}"
                else:
                    adminPrint = f"{Font.red(adminStr +' '+ f'{adminPc:^#7x}')}"

                adminStatOst = int(device.read(getStatAddr('a', mtsCoreId, 0, 0, adminId)), 16)
                adminStatOstPrint = f'{Font.red(f"{adminStatOst:^#3}")}'

            masterPc = int(device.read(getPcAddr('m', mtsCoreId, subId, clusterId, 0, 0)), 16)
            if True == isInst:
                masterInst = f"{getInstructionName(mu_inst_from_param(device, 'm', mtsCoreId, subId, clusterId, 0, 0, masterPc+ pcOffset)):^7}"
                masterPrint = f"{Font.cyan('Master : '+ masterInst)}"
            else:
                masterPrint = f"{Font.cyan(f'Master : '+ f'{masterPc:^#7x}')}"

            subIdPrint = Font.magenta(f'[{subId}]')
            clusterIdPrint = Font.magenta(f'[{clusterId}]')

            if adminPrint == None:
                print(f"{printPadding}{printPadding}{printPadding} MTS_CORE{mtsCorePrint} SUB{subIdPrint} CLUSTER{clusterIdPrint} {printPaddingHalf} {extraPrintPadding} {masterPrint}=")
            else:
                print(f"{printPaddingHalf} {adminPrint} {adminStatOstPrint} {printPaddingHalf} MTS_CORE{mtsCorePrint} SUB{subIdPrint} CLUSTER{clusterIdPrint} {printPaddingHalf} {extraPrintPadding} {masterPrint}=")
            print(f"      |", end='')
            for muId in range(0, numMuPerClst):
                print(f"    {Font.bright_black(f'S{muId}')}    |", end='')
            print()


            for threadId in range(0, MAX_THREAD_COUNT):
                print(f"  {Font.bright_black(f'T{threadId}')}  |", end='')
                if True == isInst:
                    for muId in range(0, numMuPerClst):
                        print(f" {slaveInst[get_list_offset(mtsCoreId, subId, clusterId, muId, threadId)]:^8} |", end='')
                else:
                    for muId in range(0, numMuPerClst):
                        print(f" {pcList[get_list_offset(mtsCoreId, subId, clusterId, muId, threadId)]:^#8x} |", end='')

                print()
    print()


def print_mbox_ctx(qId, mBoxCtx, mboxCtxAddr, display_is_not_empty):
    mBoxId = mBoxCtx & 0xFF
    ctxBase = (mBoxCtx >> 8) & 0xFFFF
    dataBase = (mBoxCtx >> 24) & 0xFFFF
    size = (mBoxCtx >> 40) & 0xFF
    tail = (mBoxCtx >> 48) & 0xFF
    head = (mBoxCtx >> 56) & 0xFF

    if (display_is_not_empty == True):
        if (tail == head):
            return

    addr_hex = f"{(mboxCtxAddr >> 32) & 0xFFFF:#04x}_{(mboxCtxAddr >> 16) & 0xFFFF:04x}_{mboxCtxAddr & 0xFFFF:04x}"
    mboxCtx_hex = f"{(mBoxCtx >> 48) & 0xFFFF:#04x}_{(mBoxCtx >> 32) & 0xFFFF:04x}_{(mBoxCtx >> 16) & 0xFFFF:04x}_{mBoxCtx& 0xFFFF:04x}"

    qId_print = f"[{Font.bright_cyan(f'{qId:>4}')}]"
    addr_print = f"{addr_hex}"
    MID_print = f"MID[{Font.bright_magenta(f'{mBoxId:>3}')}]"
    CB_print = f"CB[{Font.bright_magenta(f'{ctxBase:>5}')}]"
    DB_print = f"DB[{Font.bright_magenta(f'{dataBase:>5}')}]"
    SIZE_print = f"SIZE[{Font.bright_magenta(f'{size:>4}')}]"
    TAIL_print = f"T[{Font.bright_magenta(f'{tail:>3}')}]"
    HEAD_print = f"H[{Font.bright_magenta(f'{head:>3}')}]"

    print(f"{qId_print} {addr_print} : {MID_print} {CB_print} {DB_print} {SIZE_print} {TAIL_print} {HEAD_print}")

def print_mbox_infos(device, mbox_size, startAddr, name, display_is_not_empty):
    print(f"=============================== {name:^12} ===================================")
    ctx_size = mbox_size / 4
    for qId in range(0, int(ctx_size / 8)):
        mboxCtxAddr = startAddr + qId * 8
        mBoxCtx = int(device.read(mboxCtxAddr), 0)
        if (mBoxCtx != 0):
            print_mbox_ctx(qId, mBoxCtx, mboxCtxAddr, display_is_not_empty)

def mbox_print_dbg(device):
    def print_mbox_dbg(device, addr):
        v = device.read(addr)
        if (int(v, 16) != 0):
            print_mbox_ctx(0, int(v, 16), addr, False)
            

    allmu(device, print_mbox_dbg, getMboxDbgAddr)


def mbox_print_hmbox_wq(device, mtsCoreId, display_is_not_empty = False):
    print_mbox_infos(device, MBOX_SIZE_HMBOX, getMboxCtxAddr('h', mtsCoreId, 0, 0, 1), "HMBOX WQ", display_is_not_empty)

def mbox_print_hmbox_rq(device, mtsCoreId, display_is_not_empty = False):
    print_mbox_infos(device, MBOX_SIZE_HMBOX, getMboxCtxAddr('h', mtsCoreId, 0, 0, 0), "HMBOX RQ", display_is_not_empty)

def mbox_print_adm_mbox_wq(device, mtsCoreId, display_is_not_empty = False):
    print_mbox_infos(device, MBOX_SIZE_ADMIN, getMboxCtxAddr('a', mtsCoreId, 0, 0, 1), "ADMBOX WQ", display_is_not_empty)

def mbox_print_adm_mbox_rq(device, mtsCoreId, display_is_not_empty = False):
    print_mbox_infos(device, MBOX_SIZE_ADMIN, getMboxCtxAddr('a', mtsCoreId, 0, 0, 0), "ADMBOX RQ", display_is_not_empty)

def mbox_print_mts_mbox_wq(device, mtsCoreId, display_is_not_empty = False):
    for (subId, clusterId) in device.get_cluster_list():
        print_mbox_infos(device, MBOX_SIZE_MTS, getMboxCtxAddr('m', mtsCoreId, subId, clusterId, 1), "MTSBOX WQ", display_is_not_empty)

def mbox_print_mts_mbox_rq(device, mtsCoreId, display_is_not_empty = False):
    for (subId, clusterId) in device.get_cluster_list():
        print_mbox_infos(device, MBOX_SIZE_MTS, getMboxCtxAddr('m', mtsCoreId, subId, clusterId, 0), "MTSBOX RQ", display_is_not_empty)

def mbox_print_ctx_all(device, mtsCoreId, display_is_not_empty = False):
    # HMBOX WQ
    print()
    mbox_print_hmbox_wq(device, mtsCoreId, display_is_not_empty)
    mbox_print_adm_mbox_wq(device, mtsCoreId, display_is_not_empty)
    mbox_print_mts_mbox_wq(device, mtsCoreId, display_is_not_empty)

    mbox_print_hmbox_rq(device, mtsCoreId, display_is_not_empty)
    mbox_print_adm_mbox_rq(device, mtsCoreId, display_is_not_empty)
    mbox_print_mts_mbox_rq(device, mtsCoreId, display_is_not_empty)

def mu_stack(device, stack_addr, registerAddr, offset, range):
    spCsrAddr = registerAddr + 8
    spHex = device.read(spCsrAddr)

    spInt = stack_addr + int(spHex, 16) - 0x3000_0000

    print(  Font.box(f' STACK = {spHex} / REAL_ADDR = {hex(spInt)} / OFFSET = {offset:<3}', "          ").with_color(Font.blue).draw() )

    checkAddr(spInt)

    min_offset = offset - range * 8
    max_offset = offset + range * 8

    def AddPrintFunc(addr, hexValue):
        printStr = ''
        if (addr > spInt):
            printStr = f'<-- SP +{addr - spInt}'
        elif (addr == spInt):
            printStr = f'<-- SP'
        else:
            printStr = f'<-- SP {addr - spInt}'

        if (addr == (spInt + offset) ):
            print(Font.red(printStr), end='')
        else:
            print(printStr, end='')

    hexdump(device, spInt + min_offset, spInt + max_offset, length = 1,  addPrintFunc=AddPrintFunc )

def mu_stat(device, startAddr):
    PRT(device, "taskOutstandingCount", startAddr + 0, 1, [])
    PRT(device, "taskRequstCount", startAddr + 8, 1, [])
    PRT(device, "taskDoneCount", startAddr + 16, 1, [])

def mu_memory_map(device, mtsCoreId, type, subId, clusterId, muId, threadId):
    print(f'''

      << {getTypeString(type, mtsCoreId, subId, clusterId, muId, threadId) + " Memory Map"} >>

        +----------------------+
        | STACK                | <-- {readableAddr(hex(getStackAddr(type, mtsCoreId, subId, clusterId, muId, threadId)), 4, lambda x : Font.yellow(x))}
        | 0x3000_0000(16K)     |
        +----------------------+
        | DRAM                 | <-- {readableAddr(hex(getRemap4M(device, type, mtsCoreId, subId, clusterId, muId, threadId)), 4, lambda x : Font.green(x))}
        | 0x1000_0000(4M)      |
        +----------------------+
        | TEXT                 | <-- {readableAddr(hex(getTextAddr(device, type, mtsCoreId, subId, clusterId, muId, threadId)), 4, lambda x : x)}
        | 0x0000_0000(64K)     |
        +----------------------+
    ''')

def cache_error(device, startAddr):
    def filterFunc(valueStr):
        if (valueStr == '0x0000000000000000'):
            return True

    PRT(device, "MON_ERR_DATA0"             , startAddr + 0x1000, 1,
        [["ERR_ADDR", 0, 39], ["CMD_ERR", 53, 53], ["WR_OUT", 54, 54], ["RD_OUT", 55, 55], ["BRESP", 56, 57], ["RRESP", 58, 59], ["SRESP", 60, 61], ],
        filterFunc)

# ==============================================================================
# CSR Functions
# ==============================================================================


def print_l0_cache_tag(device, name, addr, array_size):
    print(hex(addr))
    for idx in range(array_size):
        arrName = name + f"_{idx:<3}"

        arrAddr = addr + idx * 8
        value_hex = device.read(arrAddr)
        dirty_bit = int(value_hex, 16) & 0x1
        address = ((int(value_hex, 16) >> 1) << 6) | 0x8_0000_0000

        data_addr = (arrAddr & ~(0x1FF)) | (((arrAddr >> 3) & 0x3F) << 6) | (1 << 12)

        print(f'{Font.bright_black(f"(0x{arrAddr:010x})")} {arrName}, {dirty_bit}, 0x{address:08x}, 0x{data_addr:08x}')


def mu_csr(device, startAddr):
    PRT(device, "MU_CSR_MON_GPI_IB"                     , startAddr + 0x0, 1, [])
    PRT(device, "MU_CSR_MON_MU_ID"                      , startAddr + 0x10, 1, [["THREAD_ID", 0, 1], ["MU_ID", 2, 4], ["MU_SUB_CLUSTER_ID", 5, 8], ["MU_TYPE", 13, 15], ])
    PRT(device, "MU_CSR_W1C_MON_PERFORMANCE"            , startAddr + 0x18, 1, [["executed_counter", 0, 31], ])
    PRT(device, "MU_CSR_REG_FETCH_STALL"                , startAddr + 0x40, 1, [])
    PRT(device, "MU_CSR_REG_REMAP_4M"                   , startAddr + 0x50, 1, [["REG_REMAP_BASE_4M_SEG", 0, 30], ["REG_REMAP_L0C_EN", 31, 31], ])
    PRT(device, "MU_CSR_MON_FETCH_TID0"                 , startAddr + 0x60, 1, [["CURRENTTHREAD_ID", 0, 31], ["MON_STID_VLD", 32, 32], ["MON_STEX_VLD", 33, 33], ["MON_STMEM_VLD", 34, 39], ["MON_WAIT_BRANCH", 40, 47], ["MON_WAIT_WRITEBACK", 48, 55], ["MON_WAIT_DMB", 56, 63], ])
    PRT(device, "MU_CSR_REG_L0C_ENABLE"                 , startAddr + 0x70, 1, [["L0C_ENABLE", 0, 0], ["L0C_HIT_ENABLE", 4, 4], ])
    PRT(device, "MU_CSR_REG_FETCH_PC"                   , startAddr + 0x80, 4, [])
    PRT(device, "MU_CSR_MON_SCHEDULE_ON"                , startAddr + 0x108, 1, [])
    PRT(device, "MU_CSR_W1C_MON_IRQ"                    , startAddr + 0x120, 1, [["IRQ_AXI_BRESP_ERROR", 0, 0], ["IRQ_AXI_RRESP_ERROR", 4, 4], ["IRQ_COP_DIV_ZERO", 8, 8], ["IRQ_COP_ERROR_TID", 12, 13], ["IRQ_SW_DEBUG_BREAK", 20, 20], ["IRQ_EXT_ERROR", 28, 28], ])
    PRT(device, "MU_CSR_MON_OST0"                       , startAddr + 0x128, 1, [["MON_WB_AXI_RL_ACT_COUNTER", 0, 7], ["MON_MU_AXI_AR_ACT_COUNTER", 8, 15], ["MON_WB_AXI_BR_ACT_COUNTER", 16, 23], ["MON_MU_AXI_AW_ACT_COUNTER", 32, 39], ])
    PRT(device, "MU_CSR_MON_OST1"                       , startAddr + 0x130, 1, [["MON_TOP_COP_RSP_ACT_COUNTER", 32, 39], ["MON_TOP_COP_REQ_ACT_COUNTER", 40, 47], ["MON_WB2GPR_COP_RSP_ACT_COUNTER", 48, 55], ["MON_MEM_COP_REQ_ACT_COUNTER", 56, 63], ])
    PRT(device, "MU_CSR_AXI_INTERNAL_AWADDR"            , startAddr + 0x140, 4, [["MON_AXI_AWADDR", 0, 39], ["MON_AXI_AWUSER", 48, 55], ["MON_AXI_AWLEN", 56, 63], ])
    PRT(device, "MU_CSR_AXI_INTERNAL_WDATA"             , startAddr + 0x160, 4, [["WDATA", 0, 63], ])
    PRT(device, "MU_CSR_AXI_INTERNAL_ARADDR"            , startAddr + 0x1a0, 4, [["MON_AXI_ARADDR", 0, 39], ["MON_AXI_ARUSER", 48, 55], ["MON_AXI_ARLEN", 56, 63], ])
    PRT(device, "MU_CSR_AXI_INTERNAL_RDATA"             , startAddr + 0x1c0, 4, [["RDATA", 0, 63], ])
    PRT(device, "MU_CSR_L0S_HAZARD_COUNT"               , startAddr + 0x200, 4, [])
    PRT(device, "MU_CSR_L0S_W_RATIO"                    , startAddr + 0x220, 1, [["wmiss_cnt", 0, 31], ["whit_cnt", 32, 63], ])
    PRT(device, "MU_CSR_L0S_R_RATIO"                    , startAddr + 0x228, 1, [["rmiss_cnt", 0, 31], ["rhit_cnt", 32, 63], ])
    PRT(device, "MU_CSR_L0S_W_BYP_CNT"                  , startAddr + 0x230, 1, [])
    PRT(device, "MU_CSR_L0S_R_BYP_CNT"                  , startAddr + 0x238, 1, [])
    PRT(device, "MU_CSR_L0S_W_TCM_CNT"                  , startAddr + 0x240, 1, [])
    PRT(device, "MU_CSR_L0S_R_TCM_CNT"                  , startAddr + 0x248, 1, [])
    PRT(device, "MU_CSR_L0S_S_CMD_CNT"                  , startAddr + 0x250, 1, [])
    PRT(device, "MU_CSR_L0S_MWR_CNT"                    , startAddr + 0x258, 1, [])
    PRT(device, "MU_CSR_L0S_MRD_CNT"                    , startAddr + 0x260, 1, [])
    PRT(device, "MU_CSR_MON_GPR_THREAD0"                , startAddr + 0x400, 32, [])
    PRT(device, "MU_CSR_MON_GPR_THREAD1"                , startAddr + 0x500, 32, [])
    PRT(device, "MU_CSR_MON_GPR_THREAD2"                , startAddr + 0x600, 32, [])
    PRT(device, "MU_CSR_MON_GPR_THREAD3"                , startAddr + 0x700, 32, [])
    # print_l0_cache_tag(device, "L0S_CSR_REG_L0S_CTAG", startAddr + 0x8000, 64)
    # PRT(device, "L0S_CSR_REG_L0S_CTAG"                  , startAddr + 0x8000, 128, [])
    # PRT(device, "L0S_CSR_REG_L0S_MEM"                   , startAddr + 0x9000, 1024, [])



def print_l1_cache_tag(device, name, addr, array_size):
    for idx in range(array_size):
        arrName = name + f"_{idx:<3}"

        arrAddr = addr + idx * 8
        value_hex = device.read(arrAddr)
        dirty_bit = int(value_hex, 16) & 0x1
        tag_data = (int(value_hex, 16) >> 1) & ((1 << 25) - 1)
        thread_id = (int(value_hex, 16) >> 26) & ((1 << 2) - 1)
        mu_id = (int(value_hex, 16) >> 28) & ((1 << 4) - 1)
        time_stamp = (int(value_hex, 16) >> 32) & ((1 << 32) - 1)

        high_addr = (tag_data) << 10
        mid_addr = ((arrAddr >> 8) & 0xF) << 6

        address = high_addr | mid_addr | 0x8_0000_0000

        data_addr = ((arrAddr & ~(0x1FFFF)) | (((arrAddr >> 3) & 0x7FF) << 6) | (1 << 18)) & ~(1 << 17)

        print(f'{Font.bright_black(f"(0x{arrAddr:010x})")} {arrName}, D:{dirty_bit}, A:0x{address:08x}, DA:0x{data_addr:08x}, TID:{thread_id}, MID:{mu_id}, TS:{time_stamp}')

def check_l1_integrity(device, name, addr, array_size):
    for idx in range(array_size):
        arrName = name + f"_{idx:<3}"

        arrAddr = addr + idx * 8
        value_hex = device.read(arrAddr)
        dirty_bit = int(value_hex, 16) & 0x1
        tag_data = (int(value_hex, 16) >> 1) & ((1 << 25) - 1)
        thread_id = (int(value_hex, 16) >> 26) & ((1 << 2) - 1)
        mu_id = (int(value_hex, 16) >> 28) & ((1 << 4) - 1)
        time_stamp = (int(value_hex, 16) >> 32) & ((1 << 32) - 1)


        high_addr = (tag_data) << 10
        mid_addr = ((arrAddr >> 8) & 0xF) << 6

        address = high_addr | mid_addr | 0x8_0000_0000

        data_addr = ((arrAddr & ~(0x1FFFF)) | (((arrAddr >> 3) & 0x7FF) << 6) | (1 << 18)) & ~(1 << 17)

        if (dirty_bit == 0 and address >= 0x8_000F_0000):
            for i in range(0, 64, 8):
                l2_data = device.read(address + i)
                l1_data = device.read(data_addr + i)
                if (l2_data != l1_data):
                    print(f'ERROR:{Font.bright_black(f"(0x{arrAddr:010x})")} {arrName}, {dirty_bit}, 0x{address + i:08x}, 0x{data_addr + i:08x}')
                    print(l1_data, l2_data)
                    break

def bit_extract(val, start, end):
    """Bitfield extraction.
    :param val: Value to extract from
    :param start: Start bit position
    :param end: End bit position
    :return: The extracted bitfield
    """
    width = end - start + 1
    offset = start

    mask = (1 << width) - 1
    return (val >> offset) & mask

def print_l1_idbg(device, name, addr, array_size):
    for idx in range(array_size):
        arrName = name + f"_{idx:<3}"

        arrAddr = addr + idx * 8
        value_hex = device.read(arrAddr)
        value = int(value_hex, 16)
        dmb = bit_extract(value, 0, 0)
        invd = bit_extract(value, 1, 1)
        flush = bit_extract(value, 2, 2)
        cache = bit_extract(value, 3, 3)
        set_id = bit_extract(value, 4, 7)
        p_id =  bit_extract(value, 8, 9)
        tid = bit_extract(value, 10, 11)
        mid = bit_extract(value, 12, 15)
        timestamp = bit_extract(value, 32, 63)

        print(f'{Font.bright_black(f"(0x{arrAddr:010x})")} {arrName}, DMB:{dmb}, INV:{invd}, FLUSH:{flush}, CACHE:{cache}, SET_ID:{set_id}, P_ID:{p_id}, TID:{tid}, MID:{mid}, TS:{timestamp}')


def l1c_csr(device, startAddr):
    PRT(device, "L1S_CSR_REG_L1S_RSQ_ON"                , startAddr + 0x10, 1, [])
    PRT(device, "L1S_CSR_REG_L1S_RIDR_ON"               , startAddr + 0x30, 1, [])
    PRT(device, "L1S_CSR_REG_L1S_FRSP_ON"               , startAddr + 0x60, 1, [])
    PRT(device, "L1S_CSR_REG_L1S_RSQ_TH"                , startAddr + 0x80, 1, [])
    PRT(device, "L1S_CSR_REG_L1S_IDLY"                  , startAddr + 0x90, 1, [])
    PRT(device, "L1S_CSR_REG_L1S_SPEC_ADDR"             , startAddr + 0xa0, 1, [])
    PRT(device, "L1S_CSR_REG_L1S_FLUSH_ADDR"            , startAddr + 0xa8, 1, [])
    PRT(device, "L1S_CSR_REG_L1S_INVD_ADDR"             , startAddr + 0xb0, 1, [])
    PRT(device, "L1S_CSR_REG_L1S_DMB_ADDR"              , startAddr + 0xb8, 1, [])
    PRT(device, "L1S_CSR_REG_L1S_CLR"                   , startAddr + 0xf8, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_ERR_DATA"              , startAddr + 0x100, 1, [["ERR_ADDR", 0, 39], ["BRESP", 56, 57], ["RRESP", 58, 59], ["SRESP", 60, 61], ])
    PRT(device, "L1S_CSR_MON_L1S_ERR_CNT"               , startAddr + 0x108, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_RSQ_WPTR"              , startAddr + 0x120, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_WCSQ_WPTR"             , startAddr + 0x130, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_RCSQ_WPTR"             , startAddr + 0x138, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_CMD_CNT"               , startAddr + 0x180, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_LOCK_OVR"              , startAddr + 0x190, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_WRAT_CNT"              , startAddr + 0x200, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_RRAT_CNT"              , startAddr + 0x208, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_SRAT_CNT"              , startAddr + 0x210, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_RCLN_CNT"              , startAddr + 0x218, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_WBYP_CNT"              , startAddr + 0x220, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_RBYP_CNT"              , startAddr + 0x228, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_SWR_CNT"               , startAddr + 0x230, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_SRD_CNT"               , startAddr + 0x238, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_MWR_CNT"               , startAddr + 0x240, 1, [])
    PRT(device, "L1S_CSR_MON_L1S_MRD_CNT"               , startAddr + 0x248, 1, [])
    PRT(device, "L1S_CSR_REG_L1S_START"                 , startAddr + 0x300, 8, [])
    PRT(device, "L1S_CSR_REG_L1S_END"                   , startAddr + 0x340, 8, [])
    PRT(device, "L1S_CSR_REG_L1S_CACHE"                 , startAddr + 0x380, 8, [["PID", 0, 1], ["Cacheable", 2, 2], ])

    #print_l1_cache_tag(device, "L1S_CSR_MEM_L1S_CTAG" , startAddr + 0x20000, 2048)
    #print_l1_idbg(device, "L1S_CSR_MEM_L1S_IDBG"  , startAddr + 0x10000, 8192)

    # check_l1_integrity(device, "L1S_CSR_MEM_L1S_CTAG" , startAddr + 0x20000, 2048)
    # PRT(device, "L1S_CSR_MEM_L1S_CTAG"                  , startAddr + 0x20000, int(2048 / 8), [])
    # PRT(device, "L1S_CSR_MEM_L1S_CMEM"                  , startAddr + 0x40000, 16384, [])

def print_l2_cache_vtag(device, name, addr, array_size):
    for idx in range(array_size):
        arrName = name + f"_{idx:<3}"

        arrAddr = addr + idx * 8
        value_hex = device.read(arrAddr)
        dirty_bit = int(value_hex, 16) & 0x1
        tag_data = int(value_hex, 16) >> 1

        way_bit_size = 5
        vset_bit_size = 5
        cache_line_bit_size = 8

        high_addr = (tag_data & ~(1 << 17)) << (vset_bit_size + cache_line_bit_size)
        mid_addr = ((arrAddr >> (way_bit_size + 3)) & ((1 << vset_bit_size) - 1) ) << cache_line_bit_size

        address = high_addr | mid_addr | 0x8_0000_0000

        vset_way_mask = 1 << (way_bit_size + vset_bit_size) - 1
        cache_set_way_mask = (1 << (way_bit_size + vset_bit_size + cache_line_bit_size)) - 1

        data_addr = ((arrAddr & ~(cache_set_way_mask)) | (((arrAddr >> 3) & (vset_way_mask) ) << cache_line_bit_size) | (1 << 20))

        print(f'{Font.bright_black(f"(0x{arrAddr:010x})")} {arrName}, RAW:{value_hex}, {dirty_bit}, 0x{address:08x}, 0x{data_addr:08x}')

def print_l2_cache_ctag(device, name, addr, array_size):
    l2_id = 0
    if ((addr - 0x220000) == getL2cAddr(1)):
        l2_id = 1

    for idx in range(array_size):
        arrName = name + f"_{idx:<3}"

        arrAddr = addr + idx * 8
        value_hex = device.read(arrAddr)
        dirty_bit = int(value_hex, 16) & 0x1
        tag_data = (int(value_hex, 16) >> 1) & ((1 << 17) - 1)

        axi_id = (int(value_hex, 16) >> 18) & ((1 << 8) - 1)
        time_stamp_low = (int(value_hex, 16) >> 26) & ((1 << 6) - 1)
        time_stamp_high = (int(value_hex, 16) >> 32) & ((1 << 26) - 1)
        time_stamp = time_stamp_high << 6 | time_stamp_low


        way_bit_size = 5
        set_bit_size = 9
        cache_line_bit_size = 8

        high_addr = (tag_data & ~((1 << 17) | (1 << 21))) << (set_bit_size + cache_line_bit_size)
        mid_addr = ((arrAddr >> (way_bit_size + 3)) & ((1 << set_bit_size) - 1) ) << cache_line_bit_size

        address = high_addr | mid_addr

        set_way_mask = (1 << (way_bit_size + set_bit_size)) - 1
        cache_set_way_mask = (1 << (way_bit_size + set_bit_size + cache_line_bit_size)) - 1

        data_addr = ((arrAddr & ~(cache_set_way_mask)) | (((arrAddr >> 3) & (set_way_mask) ) << cache_line_bit_size) | (1 << 22))

        l2_address = ((address & ~0xFF) << 1) | (l2_id << 8) | (address & 0xFF) | 0x8_0000_0000

        print(f'{Font.bright_black(f"(0x{arrAddr:010x})")} {arrName}, RAW:{value_hex}, D:{dirty_bit}, A:0x{l2_address:08x}, DA:0x{data_addr:08x}, AXI_ID:{axi_id}, TS:{time_stamp}')


def l2c_csr(device, startAddr):
    PRT(device, "L2S_CSR_REG_L2S_WBC_ON"                , startAddr + 0x0, 1, [])
    PRT(device, "L2S_CSR_REG_L2S_WBC_TH"                , startAddr + 0x8, 1, [])
    PRT(device, "L2S_CSR_REG_L2S_RSQ_ON"                , startAddr + 0x10, 1, [])
    PRT(device, "L2S_CSR_REG_L2S_RIDR_ON"               , startAddr + 0x30, 1, [])
    PRT(device, "L2S_CSR_REG_L2S_FRSP_ON"               , startAddr + 0x60, 1, [])
    PRT(device, "L2S_CSR_REG_L2S_SPEC_ADDR"             , startAddr + 0xa0, 1, [])
    PRT(device, "L2S_CSR_REG_L2S_CLR"                   , startAddr + 0xf8, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_ERR_DATA"              , startAddr + 0x100, 1, [["ERR_ADDR", 0, 39], ["BRESP", 56, 57], ["RRESP", 58, 59], ["SRESP", 60, 61], ["MRESP", 62, 63], ])
    PRT(device, "L2S_CSR_MON_L2S_ERR_CNT"               , startAddr + 0x108, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_RSQ_WPTR"              , startAddr + 0x120, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_WCSQ_WPTR"             , startAddr + 0x130, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_RCSQ_WPTR"             , startAddr + 0x138, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_CMD_CNT"               , startAddr + 0x180, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_LOCK_OVR"              , startAddr + 0x190, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_VRAT_CNT"              , startAddr + 0x1f8, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_WRAT_CNT"              , startAddr + 0x200, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_RRAT_CNT"              , startAddr + 0x208, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_SRAT_CNT"              , startAddr + 0x210, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_RDTY_CNT"              , startAddr + 0x218, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_SWR_CNT"               , startAddr + 0x230, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_SRD_CNT"               , startAddr + 0x238, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_MWR_CNT"               , startAddr + 0x240, 1, [])
    PRT(device, "L2S_CSR_MON_L2S_MRD_CNT"               , startAddr + 0x248, 1, [])
    # print_l2_cache_vtag(device, "L2S_CSR_MEM_L2S_VTAG"                  , startAddr + 0x20000, 2048)
    # PRT(device, "L2S_CSR_MEM_L2S_VMEM"                  , startAddr + 0x100000, 65535, [])
    # print_l2_cache_ctag(device, "L2S_CSR_MEM_L2S_CTAG"                  , startAddr + 0x220000, 16384)
    # PRT(device, "L2S_CSR_MEM_L2S_CMEM"                  , startAddr + 0x400000, 524288, [])


def cache_search(device):
    pass

def get_monitor_value(device, address):
    device.write(0x20000000, address)
    return device.read(0x20000008)

def print_monitor_values(str, statusValue, countValue):
    intCountValue = (int(countValue, 16))
    intValue = (int(statusValue,16))
    print("-"+str+"-")
    ar_cnt = ((intCountValue>>40)&0xFF)
    rl_cnt = ((intCountValue>>32)&0xFF)
    result = "         "
    if ar_cnt != rl_cnt :
        result = '\033[31m \033[43m' + 'MISMATCH' + '\033[0m'
    print("   "+result+"   AR:0x%02x RL:0x%02x"%(ar_cnt, rl_cnt), end ="")
    print("        || ARV[%d] ARR[%d] RV[%d] RR[%d]"%(((intValue>>44)&1),((intValue>>40)&1), ((intValue>>36)&1), ((intValue>>32)&1)))

    aw_cnt = ((intCountValue>>16)&0xFF)
    w_cnt = ((intCountValue>>8)&0xFF)
    b_cnt =  ((intCountValue)&0xFF)
    result = "         "
    if ((aw_cnt != w_cnt) or ( w_cnt != b_cnt) or (aw_cnt != b_cnt)):
        result = '\033[31m \033[43m' + 'MISMATCH' + '\033[0m'
    print("   "+result+"   AW:0x%02x W:0x%02x B:0x%02x"%(aw_cnt, w_cnt, b_cnt), end="")
    print("  || AWV[%d] AWR[%d] WV[%d] WR[%d] BV[%d] BR[%d] "%(((intValue>>20)&1), ((intValue>>16)&1), ((intValue>>12)&1), ((intValue>>8)&1), ((intValue>>4)&1),(intValue & 1)))

def bus_monitor(device):
    base_address = 0x40000 + device.cluster_id*0x1000
    print(f"===================CLUSTER {device.cluster_id} MONITOR============================")
    print(f"[MU]")
    for muId in range(device.numMuPerCluster):
        status_address = base_address + muId*2
        count_address = status_address+1
        #print(f" {muId}              |"+get_monitor_value(device, status_address)+"|"+get_monitor_value(device, count_address))
        print_monitor_values(str(muId), get_monitor_value(device, status_address), get_monitor_value(device, count_address))
    print(f"==================================================================")
    print(f'[MDMA]')
    status_address = device.cluster_id*0x1000 + 0x40106
    count_address = status_address+1
    #print(f" DRAM (0x40106) |"+get_monitor_value(device, status_address)+"|"+get_monitor_value(device, count_address))
    print_monitor_values("DRAM (0x40106)", get_monitor_value(device, status_address), get_monitor_value(device, count_address))
    status_address = device.cluster_id*0x1000 + 0x40202
    count_address = status_address+1
    #print(f" DTCM (0x40202) |"+get_monitor_value(device, status_address)+"|"+get_monitor_value(device, count_address))
    print_monitor_values("DTCM (0x40202)", get_monitor_value(device, status_address), get_monitor_value(device, count_address))
    print(f"==================================================================")
    print(f'[OTHERS]')
    status_address = device.cluster_id*0x1000 + 0x40010
    count_address = status_address+1
    #print(f" RMPM (0x40010) |"+get_monitor_value(device, status_address)+"|"+get_monitor_value(device, count_address))
    print_monitor_values("RMPM (0x40010)", get_monitor_value(device, status_address), get_monitor_value(device, count_address))
    status_address = device.cluster_id*0x1000 + 0x0
    count_address = status_address+1
    #print(f" L1-M(0x00000)  |"+get_monitor_value(device, status_address)+"|"+get_monitor_value(device, count_address))
    print_monitor_values("L1-M (0x00000)", get_monitor_value(device, status_address), get_monitor_value(device, count_address))
    status_address = device.cluster_id*0x1000 + 0x40080
    count_address = status_address+1
    #print(f" L1-S(0x40080)  |"+get_monitor_value(device, status_address)+"|"+get_monitor_value(device, count_address))
    print_monitor_values("L1-S (0x40080)", get_monitor_value(device, status_address), get_monitor_value(device, count_address))
    status_address = device.cluster_id*0x1000 + 0x40200
    count_address = status_address+1
    #print(f" CBUS (0x40200) |"+get_monitor_value(device, status_address)+"|"+get_monitor_value(device, count_address))
    print_monitor_values("CBUS (0x40200)", get_monitor_value(device, status_address), get_monitor_value(device, count_address))
    print(f"==================================================================")
