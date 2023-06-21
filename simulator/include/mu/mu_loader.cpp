//
// Created by jaewan on 22. 6. 7.
//
#include "mu.h"

extern const uint64_t _textEnd;
extern const uint64_t _sramSize;

extern uint64_t _sramDataStart;
extern uint64_t _sramDataEnd;
extern uint64_t _sramBssStart;
extern uint64_t _sramBssEnd;

extern uint64_t _dramDataStart;
extern uint64_t _dramDataEnd;
extern uint64_t _dramBssStart;
extern uint64_t _dramBssEnd;

extern uint64_t _dramReadOnlyStart;
extern uint64_t _dramReadOnlyEnd;

static constexpr uint64_t OFFSET_MU_DATA_SECTION = KB(256);
static constexpr uint64_t OFFSET_MS_DATA_SECTION = KB(16);

static constexpr uint64_t COPY_THRESHOLD = 1024;

const uint64_t DEBUG_LOG_START_ADDR = 0x900100000ull + 0x80000ull;
const uint64_t DEBUG_LOG_SIZE       = 0x1000ull;

#define ENABLE_MPU                   (0)
#define ENABLE_HOST_DEBUG_BY_TASK_ID (1)
#define ENABLE_HOST_DEBUG_BY_MU_ID   (0)

inline void copyDataSection(uint64_t* dst, uint64_t* dstEnd, uint64_t* src)
{

    uint64_t size = ((uint64_t)dstEnd - (uint64_t)dst);

    if (size > COPY_THRESHOLD)
    {
        memcpy(dst, src, size);
    }
    else
    {
        while (dst < dstEnd)
        {
            *dst++ = *src++;
        }
    }
}

void setBssSection(uint64_t* dst, uint64_t* dstEnd)
{

    uint64_t size = ((uint64_t)dstEnd - (uint64_t)dst);

    // if (size > 128)
    // {
    //     asm volatile(
    //         "mv t0, zero\t\n"
    //         "mv t1, zero\t\n"
    //         "mv t2, zero\t\n"
    //         "mv t3, zero\t\n"
    //     );

    //     while ( ((uint64_t)dstEnd - (uint64_t)dst) > 128 );
    //     {
    //         asm volatile(
    //             "sqq t0, 0(%0)\t\n"
    //             "sqq t0, 32(%0)\t\n"
    //             "sqq t0, 64(%0)\t\n"
    //             "sqq t0, 96(%0)\t\n"
    //             : : "r"(dst)
    //         );

    //         dst += 16;
    //     }
    // }
    while (dst < dstEnd)
    {
        *dst++ = 0ull;
    }
}

#ifndef _SIM_
void loader(uint64_t muType, bool readOnlyLoad)
{
    MuId_t csrMuId;
    __getCsr(MU_CSR_MU_ID, csrMuId.u16);
    uint64_t subId     = csrMuId.subId;
    uint64_t clusterId = csrMuId.clusterId;
    uint64_t muId      = csrMuId.internalMuId;

    uint64_t dataSectionAddr;
    if (muType == MU_TYPE_SLAVE)
    {
        uint64_t vClusterId = GET_REG32(MEM_START(DDR_CTRL_INFO) + offsetof(DdrCtrlInfo_t, virtualClusterIdList) + sizeof(uint32_t) * (clusterId + subId * MAX_CLUSTER_PER_SUB));
        dataSectionAddr     = MEM_START(DDR_ELF_CLST0) + MEM_SIZE(DDR_ELF_CLST0) * vClusterId + OFFSET_MU_DATA_SECTION;
    }
    else if (muType == MU_TYPE_MASTER)
    {
        uint64_t vMuId  = GET_REG32(MEM_START(DDR_CTRL_INFO) + offsetof(DdrCtrlInfo_t, virtualClusterIdList) + sizeof(uint32_t) * muId);
        dataSectionAddr = MEM_START(DDR_ELF_MASTER0) + MEM_SIZE(DDR_ELF_MASTER0) * vMuId + OFFSET_MS_DATA_SECTION;
    }
    else // MU_TYPE_ADMIN
    {
        uint64_t adminMuStartId = 32;
        dataSectionAddr         = MEM_START(DDR_ELF_ADMIN0) + MEM_SIZE(DDR_ELF_ADMIN0) * (muId - adminMuStartId) + OFFSET_MS_DATA_SECTION;
    }

    uint64_t sramElfBaseAddr         = dataSectionAddr;
    uint64_t sramSize                = (uint64_t)&_sramDataEnd - (uint64_t)&_sramDataStart;
    uint64_t dramReadOnlyElfBaseAddr = ALIGN_UP(sramElfBaseAddr + sramSize, 4);

    uint64_t dramReadOnlySize = (uint64_t)&_dramReadOnlyEnd - (uint64_t)&_dramReadOnlyStart;
    uint64_t dramElfBaseAddr  = ALIGN_UP(dramReadOnlyElfBaseAddr + dramReadOnlySize, 4);

    copyDataSection(&_sramDataStart, &_sramDataEnd, (uint64_t*)(sramElfBaseAddr));
    setBssSection(&_sramBssStart, &_sramBssEnd);

    copyDataSection(&_dramDataStart, &_dramDataEnd, (uint64_t*)(dramElfBaseAddr));
    setBssSection(&_dramBssStart, &_dramBssEnd);

    if (readOnlyLoad)
    {
        copyDataSection(&_dramReadOnlyStart, &_dramReadOnlyEnd, (uint64_t*)(dramReadOnlyElfBaseAddr));
    }
}
#endif

void setStartDebugLog(MuHeader muHeader)
{
#if (_SIM_ == 0)
    #if (ENABLE_HOST_DEBUG_BY_TASK_ID == 1)
    {
        uint64_t         curCycleCnt;
        MxTaskDbgInfo_t* debugInfoByTask = getTaskDbgInfoByHeader(muHeader);
        uint64_t         execCycleCount;
        uint64_t         CycleCount;

        __getCycle(CycleCount);
        __getInstructionCount(execCycleCount);

        debugInfoByTask->slaveExecStartCycle = execCycleCount;
        debugInfoByTask->slaveStartCycle     = CycleCount;
    }
    #endif

    #if (ENABLE_HOST_DEBUG_BY_MU_ID == 1)
    {
        uint64_t       curCycleCnt;
        uint64_t       logId           = (muHeader.subId << 7) | (muHeader.clusterId << 5) | (muHeader.internalMuId << 2) | (muHeader.threadId);
        MuDebugInfo_t* debugInfoByMuId = (MuDebugInfo_t*)REAL_MEM(DEBUG_LOG_START_ADDR + DEBUG_LOG_SIZE * logId);
        MxTaskCmd_t*   taskInfo        = getTaskCmdByHeader(muHeader);
        __getCycle(curCycleCnt);
        debugInfoByMuId->startTimeStamp = curCycleCnt;
        debugInfoByMuId->taskId         = muHeader.taskId;
        debugInfoByMuId->inputBufAddr   = taskInfo->taskBufInfo.inputParamAddr;
        debugInfoByMuId->inputBufSize   = taskInfo->taskBufInfo.inputParamSize;
    }
    #endif

#endif
}

void setEndDebugLog(MuHeader muHeader)
{
#if (_SIM_ == 0)
    #if (ENABLE_HOST_DEBUG_BY_TASK_ID == 1)
    {
        uint64_t         execCycleCount;
        MxTaskDbgInfo_t* debugInfoByTask = getTaskDbgInfoByHeader(muHeader);
        uint64_t         CycleCount;

        __getCycle(CycleCount);
        __getInstructionCount(execCycleCount);

        debugInfoByTask->slaveExecEndCycle = execCycleCount;
        debugInfoByTask->slaveEndCycle     = CycleCount;
    }
    #endif

    #if (ENABLE_HOST_DEBUG_BY_MU_ID == 1)
    {
        uint64_t       curCycleCnt;
        uint64_t       logId           = (muHeader.subId << 7) | (muHeader.clusterId << 5) | (muHeader.internalMuId << 2) | (muHeader.threadId);
        MuDebugInfo_t* debugInfoByMuId = (MuDebugInfo_t*)REAL_MEM(DEBUG_LOG_START_ADDR + DEBUG_LOG_SIZE * logId);
        __getCycle(curCycleCnt);
        debugInfoByMuId->endTimeStamp = curCycleCnt;
    }
    #endif

#endif
}

void setMpu(MuHeader muHeader)
{
#if (_SIM_ == 0 && ENABLE_MPU == 1)
    const uint32_t MPU_THREAD0_CHECKER_ON            = 0x0A0;
    const uint32_t MPU_THREAD0_ERROR_OFFSET_END0     = 0x0C0;
    const uint32_t MPU_THREAD0_ERROR_OFFSET_START1   = 0x0C8;
    const uint32_t MPU_THREAD0_CHECKER_REG_SIZE      = 0x08;
    const uint32_t MPU_THREAD0_ERROR_OFFSET_REG_SIZE = 0x10;

    uint64_t     cBusBaseAddr = MEM_START(MTS_SUB0);
    uint64_t     checkerThreadOffset;
    uint64_t     errorOffsetThreadOffset;
    uint64_t     end0Addr;
    uint64_t     start1Addr;
    uint64_t     dummy;
    MxTaskCmd_t* taskInfo = getTaskCmdByHeader(muHeader);

    cBusBaseAddr += MEM_SIZE(MTS_SUB0) * muHeader.subId;
    cBusBaseAddr += MEM_SIZE(CLST0) * muHeader.clusterId;
    cBusBaseAddr += MEM_SIZE(MU_S0_CTRL) * muHeader.internalMuId;

    checkerThreadOffset     = MPU_THREAD0_CHECKER_REG_SIZE * muHeader.threadId;
    errorOffsetThreadOffset = MPU_THREAD0_ERROR_OFFSET_REG_SIZE * muHeader.threadId;

    uint64_t taskStartAddr = taskInfo->taskBufInfo.inputParamAddr;
    uint64_t taskEndAddr   = ALIGN_UP((taskStartAddr + taskInfo->taskBufInfo.inputParamSize), KB(4));
    __cwrite((cBusBaseAddr + MPU_THREAD0_CHECKER_ON + checkerThreadOffset), 0);
    __cwrite((cBusBaseAddr + MPU_THREAD0_ERROR_OFFSET_END0 + errorOffsetThreadOffset), (taskStartAddr >> 12));
    __cwrite((cBusBaseAddr + MPU_THREAD0_ERROR_OFFSET_START1 + errorOffsetThreadOffset), (taskEndAddr >> 12));
    __cwrite((cBusBaseAddr + MPU_THREAD0_CHECKER_ON + checkerThreadOffset), 1);
    __cread((cBusBaseAddr + MPU_THREAD0_CHECKER_ON + checkerThreadOffset), dummy);
#endif
}

void invdCache(void)
{
    invdL0Pool(L0_POOL1_DDR_HOST_DATA);
#if (TASK_INPUT_L1S_CACHE_ON == 1)
    invdL1Pool(L1_POOL1_MU_DATA_1);
#endif
    flushL1Pool(L1_POOL2_TASK_OUTPUT);
}