#pragma once
#include "csr/cctrl_csr.h"
///////////////////////////////////////////////////////////////////
//
//   +-----------+-------------+-----------+------------+
//	 |  MU Type  |  Cluster ID |   MU ID   |  Thread ID |
//   +-----------+-------------+-----------+------------+
//
///////////////////////////////////////////////////////////////////

enum
{
    DDR_INFO_0_32GB,
    DDR_INFO_1_64GB,
} DdrInfo_e;

#ifdef __cplusplus
typedef union
{
	uint16_t u16;
	struct{
		uint16_t threadId : 2;
		uint16_t internalMuId : 6;
		uint16_t clusterId : 3;
		uint16_t subId : 3;
		uint16_t : 2;
	};
    struct
    {
        uint64_t clusterMuId : 8;
		uint64_t : 56;
    };
} MuId_t;

union MuHeader
{
	MuHeader() {}
	inline MuHeader(const uint64_t& value) {u64 = value;}
    inline MuHeader(const uint64_t clusterId, uint64_t internalMuId, uint64_t threadId)
    {
        u64 = 0;
        this->clusterId = clusterId;
        this->internalMuId = internalMuId;
        this->threadId = threadId;
    }
	uint64_t u64;
	struct
	{
		uint64_t muId : 16;
        uint64_t rvsd : 2;
		uint64_t fixedPhyMuId : 1;
		uint64_t muOpcode : 8;
		uint64_t rsvd2 : 5;
		uint64_t hostOpcode : 4;
		uint64_t taskId : 16;
		uint64_t jobId : 6;
		uint64_t mtsCoreId : 2;
		uint64_t fpgaId : 4;
	};
	struct
	{
		uint64_t threadId : 2;
		uint64_t internalMuId : 6;
		uint64_t clusterId : 3;
		uint64_t subId : 3;
		uint64_t : 50;
	};
    struct
    {
        uint64_t clusterMuId : 8;
		uint64_t : 56;
    };

};
#endif


typedef union
{
    uint64_t statusMonitor; //Read only
    struct {
        uint64_t regIrq:1;           //1
        uint64_t :7;                          //8
        uint64_t ackCnt:4;           //12
        uint64_t :4;                          //16
        uint64_t irqPin:1;           //17
        uint64_t :3;                          //20
        uint64_t ackPin:1;           //21
        uint64_t :3;                          //24
        uint64_t msiPinEnable:1;        //25
        uint64_t msixPinEnable:1;       //26
        uint64_t :2;                          //28
        uint64_t msixVectorPin:3;      //31
        uint64_t :33;                         //64
    };
} PcieIrqStatus_t;

typedef struct
{
    PcieIrqStatus_t status;
    uint64_t setReqIrq; // Write any (generally write 0x1 or 0xf)
    uint64_t clearRegIrq;
    uint64_t clearAckCnt;
} PcieIrq_t;


typedef struct
{
	PcieIrq_t irq[16];
} PcieIrqReg_t;


typedef struct 
{
    uint64_t inputParamAddr;
    uint64_t inputParamSize;
    uint64_t outputBufAddr;
    uint64_t outputBufSize;
}TaskBufInfo_t;

typedef union
{
    uint64_t qw[8];
    struct
    {
        TaskBufInfo_t taskBufInfo;
        uint64_t adminCmdOpcode : 4;
        uint64_t numSlaveMu : 4;
        uint64_t rsvd : 58;
        uint64_t threadBitmap;  //qw4
        uint64_t argc;
        uint64_t rsvd2;
    };
} MxTaskCmd_t;

typedef union
{
    uint64_t qw[8];
    struct 
    {
        uint32_t outputOverFlow;
        uint32_t adminOutstandingCnt;
        uint32_t adminStartCycle;
        uint32_t adminEndCycle;
        uint32_t masterStartCycle;
        uint32_t masterEndCycle;
        uint32_t slaveStartCycle;
        uint32_t slaveEndCycle;
        uint32_t slaveExecStartCycle;
        uint32_t slaveExecEndCycle;
        uint64_t rsvd0;
        uint64_t rsvd1;
        uint64_t rsvd2;
    };
}MxTaskDbgInfo_t;



typedef struct
{
	uint64_t taskOutstandingCount;
	uint64_t taskRequstCount;
	uint64_t taskDoneCount;
	uint64_t rsvd;
} MuStat_t;


typedef union
{
    struct
    {
        uint64_t mid : 8;
        uint64_t ctxBase : 16;
        uint64_t dataBase : 16;
        uint64_t size : 8;
        uint64_t tail : 8;
        uint64_t head : 8;
    };
    uint64_t u64;
    uint32_t u32[2];
} MboxContext_t;
static_assert(sizeof(MboxContext_t) == sizeof(uint64_t), "MboxContextSize");


typedef union 
{
    uint64_t u64;
    struct 
    {
        uint64_t data : 32;
        uint64_t allocatedTid : 10;
        uint64_t rsvd0 : 5;
        uint64_t allocated : 1;
        uint64_t failedTid : 10;
        uint64_t rsvd1 : 5;
        uint64_t fail : 1;
    };
}GMonCtx_t;

typedef union
{
    uint32_t dw;
struct 
    {
        uint32_t addrLsb : 3;
        uint32_t addr : 9;
        uint32_t tid : 10;
        uint32_t mirror : 1;
        uint32_t : 9;
    };
}GMonCmd_t;

typedef struct
{
    CctrlCsr_t cctrlCsr;
    uint32_t virtualClusterIdList[MAX_CLUSTER_PER_CORE];
    uint8_t padding0[1024 - sizeof(CctrlCsr_t) - sizeof(virtualClusterIdList)];
    uint64_t remappingDramAddrList[MAX_CLUSTER_PER_CORE];
    uint8_t padding1[512 - sizeof(remappingDramAddrList)];
} DdrCtrlInfo_t;
static_assert(sizeof(DdrCtrlInfo_t) <= MEM_SIZE(DDR_CTRL_INFO), "DdrCtrlInfoSize");

class MuBianry
{
public:
    MuBianry(uint32_t* text, uint32_t textSize, uint32_t* textLib, uint32_t textLibSize, uint32_t* rodata, uint32_t dataSize)
    {
        ptrText = text;
        ptrTextLib = textLib;
        ptrRodata = rodata;

        sizeText = textSize;
        sizeTextLib = textLibSize;
        sizeRodata = dataSize;
    };

    uint32_t* ptrText;
    uint32_t* ptrTextLib;
    uint32_t* ptrRodata;

    uint32_t sizeText;
    uint32_t sizeTextLib;
    uint32_t sizeRodata;
};

