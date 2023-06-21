#pragma once

inline MxTaskCmd_t* getTaskCmdByHeader(MuHeader& muHeader)
{
    // TO-DO : L0C Pool1 addr bit set if isMu == true
    return (MxTaskCmd_t*)REAL_MEM(MEM_START(DDR_TASK_CMD) + sizeof(MxTaskCmd_t) * muHeader.taskId);
}


inline MxTaskDbgInfo_t* getTaskDbgInfoByHeader(MuHeader& muHeader)
{
    // TO-DO : L0C Pool1 addr bit set if isMu == true
    return (MxTaskDbgInfo_t*)REAL_MEM(MEM_START(DDR_TASK_DEBUG_INFO) + sizeof(MxTaskDbgInfo_t) * muHeader.taskId);
}

inline uint64_t GET_REMAP_ADDR(uint64_t addr, MuId_t muId)
{
    // This function only works on slave mu
    uint64_t ramapAddr = addr;

    uint64_t threadId = muId.threadId;
    uint64_t slaveMuId = muId.internalMuId;
    uint64_t clusterId = muId.clusterId;
    uint64_t subId = muId.subId;

    uint64_t vClusterId = GET_REG32(MEM_START(DDR_CTRL_INFO) + offsetof(DdrCtrlInfo_t, virtualClusterIdList) + sizeof(uint32_t) * (clusterId + subId * MAX_CLUSTER_PER_SUB));

    if (IS_MEM_RANGE(MU_DRAM_V_ADDR, addr))
    {
        ramapAddr = MEM_START(DDR_SLAVE_MEM_CLST0)
                        + MEM_SIZE(DDR_SLAVE_MEM_CLST0) * vClusterId
                        + DRAM_PER_THREAD * (slaveMuId * NUM_THREAD_PER_MU + threadId)
                        + addr - MEM_START(MU_DRAM_V_ADDR);
    }
    else if(IS_MEM_RANGE(MU_DTCM_V_ADDR, addr))
    {
        ramapAddr = MEM_START(CLST0)
                        + MEM_SIZE(MTS_SUB0) * subId
                        + MEM_SIZE(CLST0) * clusterId
                        + MEM_SIZE(MU_S0_CTRL) * slaveMuId
                        + 0x10000/*DTCM OFFSET*/
                        + KB(16)/*DTCM SIZE*/ * threadId
                        + addr - MEM_START(MU_DTCM_V_ADDR);
    }

    return ramapAddr;
}

// For L1 Cache
inline void specL1(uint64_t addr)
{
    SET_REG((L1_SPEC_ADDR | MASK_L0_SKIP_AGGR), addr);
}

inline void flushL1(uint64_t addr)
{
    for(uint64_t setIdx = 0 ; setIdx < NUM_L1_SET; setIdx++)
    {
        SET_REG((L1_FLUSH_ADDR | MASK_L0_SKIP_AGGR), (addr + (BYTE_L1_CACHE_LINE * setIdx)));
    }
}

inline void invdL1(uint64_t addr)
{
    for(uint64_t setIdx = 0 ; setIdx < NUM_L1_SET; setIdx++)
    {
        SET_REG((L1_INVD_ADDR | MASK_L0_SKIP_AGGR), (addr + (BYTE_L1_CACHE_LINE * setIdx)));
    }
}

inline void dmbL1(void)
{
    SET_REG((L1_DMB_ADDR | MASK_L0_SKIP_AGGR), 1/*any value*/);
}

inline uint64_t _getAddrOnL1Pool(uint64_t pool)
{
    switch (pool)
    {
    case L1_POOL0_MU_DATA_0:
        return (MEM_START(DDR_SLAVE_MEM_CLST0_MU0));
    case L1_POOL1_MU_DATA_1:
        return (MEM_START(DDR_TASK_INPUT));
    case L1_POOL2_TASK_OUTPUT:
        return (MEM_START(DDR_TASK_OUTPUT));
    case L1_POOL3_HOST_HEAP:
        return (MEM_START(DDR_HOST_HEAP));
    default:
        return 0;
    }
}

inline void invdL1Pool(uint64_t pool)
{
    #if (_SIM_ == 0)
    invdL1(_getAddrOnL1Pool(pool));
    dmbL1();
    __dmb();
    #endif
}

inline void flushL1Pool(uint64_t pool)
{
    #if (_SIM_ == 0)
    flushL1(_getAddrOnL1Pool(pool));
    dmbL1();
    __dmb();
    #endif
}

inline void invdL0Pool(uint64_t pool)
{
    #if (_SIM_ == 0)
    #if (ENABLE_L0_CACHE)
    __invdL0(pool);
    __dmb();
    #endif
    #endif
}

inline void flushL0Pool(uint64_t pool)
{
    #if (_SIM_ == 0)
    #if (ENABLE_L0_CACHE)
    __flushL0(pool);
    __dmb();
    #endif
    #endif
}

inline void clearL1PoolDbgPool(void)
{
    #if (_SIM_ == 0)
    #if (CLEAR_L1S_TEST)
    uint64_t pushCount = 8;
    uint64_t startAddr = MEM_START(DDR_ELF_MASTER0) + MEM_SIZE(DDR_ELF_MASTER0) * 10;
    uint64_t curAddr = startAddr;
    uint64_t cacheLineCnt = NUM_L1_SET * NUM_L1_WAY * pushCount;   // 512 * pushCount

    for (uint64_t idx = 0; idx < cacheLineCnt; idx++)
    {
        curAddr = startAddr + idx * BYTE_L1_CACHE_LINE;
        //SET_REG(curAddr, 1);
        volatile uint64_t readValue = GET_REG(curAddr);
    }

    __dmb();
    #endif
    #endif
}

inline GMonCtx_t acquireLock(uint64_t tid, uint64_t addr)
{
    GMonCmd_t gmonCmd;
    gmonCmd.dw = 0;
    gmonCmd.tid = tid;
    gmonCmd.addr = addr;

    uint64_t gmonCmdAddr = MEM_START(GMON) | gmonCmd.dw;
    GMonCtx_t ret = {0,};
    do
    {
#if (_SIM_ == 0)
        __cread(gmonCmdAddr, ret.u64);
#else
        ret.u64 = *((volatile uint64_t*)(gmonCmdAddr));
#endif
    } while (ret.allocated != 1);

    return ret;
}

inline void returnLock(uint64_t tid, uint64_t addr, uint32_t val)
{
    GMonCmd_t gmonCmd;
    gmonCmd.dw = 0;
    gmonCmd.tid = tid;
    gmonCmd.addr = addr;

    uint64_t gmonCmdAddr = MEM_START(GMON) | gmonCmd.dw;

#if (_SIM_ == 0)
    __cwrite(gmonCmdAddr, val);
#else
    *((volatile uint64_t*)(gmonCmdAddr)) = val;
#endif    
}

inline GMonCtx_t getGlobalCtm(uint64_t addr)
{
    GMonCmd_t gmonCmd;
    gmonCmd.dw = 0;
    gmonCmd.addr = addr;
    gmonCmd.mirror = 1;

    uint64_t gmonCmdAddr = MEM_START(GMON) | gmonCmd.dw;
    GMonCtx_t ret;

#if (_SIM_ == 0)
    __cread(gmonCmdAddr, ret.u64);
#else
    ret.u64 = *((volatile uint64_t*)(gmonCmdAddr));
#endif    

    return ret;
}

inline void setGlobalCtm(uint64_t addr, uint32_t val)
{
    GMonCmd_t gmonCmd;
    gmonCmd.dw = 0;
    gmonCmd.addr = addr;
    gmonCmd.mirror = 1;

    uint64_t gmonCmdAddr = MEM_START(GMON) | gmonCmd.dw;

#if (_SIM_ == 0)
    __cwrite(gmonCmdAddr, val);
#else    
    *((volatile uint64_t*)(gmonCmdAddr)) = val;
#endif    
}

inline uint64_t getGmonTid(MuHeader muHeader)
{
    uint64_t tid = (muHeader.clusterMuId & 0x3f) | (muHeader.clusterId << 6) | (muHeader.subId << 8);
    return tid;
}