from deviceif import *

MAX_ADMIN_MU_COUNT = 32
MAX_MASTER_MU_COUNT = 32
MAX_CLUSTER_PER_SUB_COUNT = 8
MAX_SUB_COUNT = 4
MAX_THREAD_COUNT = 4
MAX_SLAVE_MU_COUNT_PER_CLUSTER = 8

MU_SFR_BASE_ADDR = 0x00_8000_0000
MU_SFR_SUB_SIZE = 0x00_1000_0000
MU_SFR_CLUSTER_SIZE = 0x00_0200_0000
MU_SFR_SIZE = 0x00_0004_0000

MS_SFR_BASE_ADDR = 0x00_C200_0000
MS_SFR_SIZE = 0x00_0004_0000
ADMIN_MS_OFFSET = 32 * MS_SFR_SIZE

L1C_ADDR_OFFSET = 0x00_0128_0000

METIS_CORE_DDR_BASE_OFFSET = 0x10_0000_0000
METIS_CORE_SFR_BASE_OFFSET = 0x1_0000_0000

DDR_BASE_ADDR = 0x8_0000_0000
DDR_ELF_SIZE = 0x0_0010_0000
DDR_ELF_MU_BASE_ADDR = 0xF_FDF0_0000
DDR_ELF_MASTER_MS_BASE_ADDR = 0xF_FEF0_0000
DDR_ELF_ADMIN_MS_BASE_ADDR = 0x8_1600_0000

DDR_MU_DATA_ADDR_BASE = 0x8_4000_0000
DDR_MU_DATA_SIZE_PER_THREAD = 4 * 1024 * 1024

DDR_MASTER_MS_DATA_ADDR_BASE = 0x810000000
DDR_MASTER_MS_DATA_SIZE = 4 * 1024 * 1024

DDR_ADMIN_MS_DATA_ADDR_BASE = 0xA_3000_0000
DDR_ADMIN_MS_DATA_SIZE = 4 * 1024 * 1024


MBOX_SIZE_HMBOX = 16 * 1024
MBOX_SIZE_ADMIN = 128 * 1024
MBOX_SIZE_MTS = 4 * 1024

MBOX_BASE_ADDR_HMBOX = 0x00_C100_0000
MBOX_BASE_ADDR_ADMIN = 0x00_C300_0000
MBOX_BASE_ADDR_MTS_OFFSET = 0x00_0100_0000

DTCM_BASE_OFFSET = 0x10000

DDR_MU_CTRL_INFO = 0x8_0200_0000


def MB(v):
    return v * 1024 * 1024

def KB(v):
    return v * 1024

def getMuCsrAddr(type, mtsCoreId, subId, clusterId, muId):
    if (type == 's'):
        return MU_SFR_BASE_ADDR + subId * MU_SFR_SUB_SIZE + clusterId * MU_SFR_CLUSTER_SIZE + muId * MU_SFR_SIZE + mtsCoreId * METIS_CORE_SFR_BASE_OFFSET
    elif (type == 'm'):
        muId = (subId << 3) | clusterId
        return MS_SFR_BASE_ADDR + muId * MS_SFR_SIZE + mtsCoreId * METIS_CORE_SFR_BASE_OFFSET
    elif (type == 'a'):
        return MS_SFR_BASE_ADDR + ADMIN_MS_OFFSET + muId * MS_SFR_SIZE + mtsCoreId * METIS_CORE_SFR_BASE_OFFSET

    assert(0)

def getMboxCtxAddr(type, mtsCoreId, subId, clusterId, isWq):
    if (type == 'h'):
        if (isWq):
            return MBOX_BASE_ADDR_HMBOX
        else:
            return MBOX_BASE_ADDR_HMBOX + int(MBOX_SIZE_HMBOX / 4)
    elif (type == 'a'):
        if (isWq):
            return MBOX_BASE_ADDR_ADMIN
        else:
            return MBOX_BASE_ADDR_ADMIN + int(MBOX_SIZE_ADMIN / 4)
    elif (type == 'm'):
        if (isWq):
            return getMuCsrAddr('s', mtsCoreId, subId, clusterId, 0) + MBOX_BASE_ADDR_MTS_OFFSET
        else:
            return getMuCsrAddr('s', mtsCoreId, subId, clusterId, 0) + MBOX_BASE_ADDR_MTS_OFFSET + int(MBOX_SIZE_MTS / 4)

    assert(0)

def getMboxDbgAddr(type, mtsCoreId, subId, clusterId, muId):
    if (type == 'm'):
        return mtsCoreId * 0x1_0000_0000 +  MBOX_BASE_ADDR_ADMIN + MBOX_SIZE_ADMIN - 8
    if (type == 's'):
        return getMuCsrAddr('s', mtsCoreId, subId, clusterId, 0) + MBOX_BASE_ADDR_MTS_OFFSET + MBOX_SIZE_MTS - 8
    return 0
#TO-DO : UPDATE STAT ADDR

def getStatAddr(type, mtsCoreId, subId, clusterId, muId):

    csrBaseAddr = getMuCsrAddr(type, mtsCoreId, subId, clusterId, muId)
    if (type == 's'):
        return 0x0
    elif (type == 'm'):
        return csrBaseAddr + DTCM_BASE_OFFSET
    elif (type == 'a'):
        return csrBaseAddr + DTCM_BASE_OFFSET

    assert(0)

def getRegisterAddr(type, mtsCoreId, subId, clusterId, muId, threadId, index):
    addr = getMuCsrAddr(type, mtsCoreId, subId, clusterId, muId)
    if (addr == 0):
        return 0
    if (type == 's'):
        return addr + 0x400 + threadId * 0x100 + index * 8
    elif (type == 'm'):
        return addr + 0x400 + index * 8
    elif (type == 'a'):
        return addr + 0x400 + index * 8

    assert(0)

def getPcAddr(type, mtsCoreId, subId, clusterId, muId, threadId):
    addr = getMuCsrAddr(type, mtsCoreId, subId, clusterId, muId)
    if (addr == 0):
        return 0
    if (type == 's'):
        return addr + 0x80 + threadId * 8
    elif (type == 'm'):
        return addr + 0x80
    elif (type == 'a'):
        return addr + 0x80

    assert(0)

def get4ByteData(device, addr):
    data = int(device.read(addr & ~7), 16)

    if (addr % 8 == 0):
        data = data & 0xFFFFFFFF
    else:
        data = (data >> 32) & 0xFFFFFFFF
    
    return data

def getTextAddr(device, mtsCoreId, type, subId, clusterId, muId, threadId):
    REMAP_TABLE_BASE_ADDR = 0x802000028 + mtsCoreId * METIS_CORE_SFR_BASE_OFFSET
    DATA_SIZE = 4

    if (type == 's'):
        targetId = (clusterId + subId * MAX_CLUSTER_PER_SUB_COUNT)
        RemappedTargetId = get4ByteData(device, REMAP_TABLE_BASE_ADDR + (targetId * DATA_SIZE))
        return DDR_ELF_MU_BASE_ADDR + RemappedTargetId * DDR_ELF_SIZE
    elif (type == 'm'):
        targetId = (clusterId + subId * MAX_CLUSTER_PER_SUB_COUNT)
        RemappedTargetId = get4ByteData(device, REMAP_TABLE_BASE_ADDR + (targetId * DATA_SIZE))
        return DDR_ELF_MASTER_MS_BASE_ADDR + RemappedTargetId * DDR_ELF_SIZE
    elif (type == 'a'):
        return DDR_ELF_ADMIN_MS_BASE_ADDR + muId * DDR_ELF_SIZE

    assert(0)



#TO-DO gerRemap4M
#TO-DO add mtsCoreId
def getRemap4M(device, type, mts_core_id, subId, clusterId, muId, threadId):
    if (type == 's'):
        target_addr = DDR_MU_CTRL_INFO + 512 + (clusterId + subId * MAX_CLUSTER_PER_SUB_COUNT) * 8
        cluster_addr = int(device.read(target_addr), 16)

        addr = cluster_addr + DDR_MU_DATA_SIZE_PER_THREAD * (muId * MAX_THREAD_COUNT + threadId) + mts_core_id * METIS_CORE_DDR_BASE_OFFSET
        return addr
    elif (type == 'm'):
        muId = (subId << 3) | clusterId
        return DDR_MASTER_MS_DATA_ADDR_BASE + muId * DDR_MASTER_MS_DATA_SIZE + mts_core_id * METIS_CORE_DDR_BASE_OFFSET
    elif (type == 'a'):
        return DDR_ADMIN_MS_DATA_ADDR_BASE + muId * DDR_ADMIN_MS_DATA_SIZE + mts_core_id * METIS_CORE_DDR_BASE_OFFSET

    assert(0)


#TO-DO DTCM?
def getStackAddr(type, mtsCoreId, subId, clusterId, muId, threadId):
    return getMuCsrAddr(type, mtsCoreId, subId, clusterId, muId) + 0x10000 + threadId * 16 * 1024

def getL1cAddr(mtsCoreId, subId, clusterId):
    return mtsCoreId * METIS_CORE_SFR_BASE_OFFSET + MU_SFR_BASE_ADDR + subId * MU_SFR_SUB_SIZE + clusterId * MU_SFR_CLUSTER_SIZE + L1C_ADDR_OFFSET

def getL2cAddr(mtsCoreId, index):
    return mtsCoreId * METIS_CORE_SFR_BASE_OFFSET + 0x00_D000_0000 + index * MB(32)

def checkAddr(addr):
    assert(addr >= 0x00_1FFF_FFFF)

def IsUartOnly(addr):
    return 0x2000_0000 <= addr and addr <= 0x02FFF_FFFF
