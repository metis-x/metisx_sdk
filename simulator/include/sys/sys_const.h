#ifndef _SYS_CONST_H_
#define _SYS_CONST_H_

enum
{
    MU_TYPE_SLAVE,
    MU_TYPE_MASTER,
    MU_TYPE_ADMIN,
} MuType_e;

enum
{
	HOST_OPCODE_INIT_MASTER_BINARY   = 0,
	HOST_OPCODE_LOAD_MASTER_BINARY   = 1,
	HOST_OPCODE_LOAD_SLAVE_BINARY    = 2,
	HOST_OPCODE_SET_THREAD_BITMAP	 = 3,
} HostOpcode_e;

enum MASTER_MS_MBOX_QID
{
    MASTER_MS_QID_CLST_START = 0,
    MASTER_MS_QID_MU0_TH0 = MASTER_MS_QID_CLST_START,
	MASTER_MS_QID_MU0_TH1,
	MASTER_MS_QID_MU0_TH2,
	MASTER_MS_QID_MU0_TH3,
	MASTER_MS_QID_MU1_TH0,
	MASTER_MS_QID_MU1_TH1,
	MASTER_MS_QID_MU1_TH2,
	MASTER_MS_QID_MU1_TH3,
	MASTER_MS_QID_MU2_TH0,
	MASTER_MS_QID_MU2_TH1,
	MASTER_MS_QID_MU2_TH2,
	MASTER_MS_QID_MU2_TH3,
	MASTER_MS_QID_MU3_TH0,
	MASTER_MS_QID_MU3_TH1,
	MASTER_MS_QID_MU3_TH2,
	MASTER_MS_QID_MU3_TH3,
	MASTER_MS_QID_MU4_TH0,
	MASTER_MS_QID_MU4_TH1,
	MASTER_MS_QID_MU4_TH2,
	MASTER_MS_QID_MU4_TH3,
	MASTER_MS_QID_MU5_TH0,
	MASTER_MS_QID_MU5_TH1,
	MASTER_MS_QID_MU5_TH2,
	MASTER_MS_QID_MU5_TH3,
	MASTER_MS_QID_MU6_TH0,
	MASTER_MS_QID_MU6_TH1,
	MASTER_MS_QID_MU6_TH2,
	MASTER_MS_QID_MU6_TH3,
	MASTER_MS_QID_MU7_TH0,
	MASTER_MS_QID_MU7_TH1,
	MASTER_MS_QID_MU7_TH2,
	MASTER_MS_QID_MU7_TH3,
	MASTER_MS_QID_MU8_TH0,
	MASTER_MS_QID_MU8_TH1,
	MASTER_MS_QID_MU8_TH2,
	MASTER_MS_QID_MU8_TH3,
	MASTER_MS_QID_MU9_TH0,
	MASTER_MS_QID_MU9_TH1,
	MASTER_MS_QID_MU9_TH2,
	MASTER_MS_QID_MU9_TH3,
	MASTER_MS_QID_MU10_TH0,
	MASTER_MS_QID_MU10_TH1,
	MASTER_MS_QID_MU10_TH2,
	MASTER_MS_QID_MU10_TH3,
	MASTER_MS_QID_MU11_TH0,
	MASTER_MS_QID_MU11_TH1,
	MASTER_MS_QID_MU11_TH2,
	MASTER_MS_QID_MU11_TH3,
    MASTER_MS_QID_CLST_END = MASTER_MS_QID_MU11_TH3,
	NUM_MASTER_MS_QID_CLST,

    MASTER_MS_QID_ADMIN = 61,
    MASTER_MS_QID_BLAZE_FOR_MU_PRINT = 62,
    MASTER_MS_QID_BLAZE_FOR_MS_PRINT = 63,
};

enum ADMIN_MS_MBOX_QID
{
	ADMIN_MS_QID_HOST,
	ADMIN_MS_QID_BLAZE,
	ADMIN_MS_QID_MASTER0,
	ADMIN_MS_QID_MASTER1,
	ADMIN_MS_QID_MASTER2,
	ADMIN_MS_QID_MASTER3,
	ADMIN_MS_QID_MASTER4,
	ADMIN_MS_QID_MASTER5,
	ADMIN_MS_QID_MASTER6,
	ADMIN_MS_QID_MASTER7,
	ADMIN_MS_QID_MASTER8,
	ADMIN_MS_QID_MASTER9,
	ADMIN_MS_QID_MASTER10,
	ADMIN_MS_QID_MASTER11,
	ADMIN_MS_QID_MASTER12,
	ADMIN_MS_QID_MASTER13,
	ADMIN_MS_QID_MASTER14,
	ADMIN_MS_QID_MASTER15,
	
	ADMIN_MS_QID_BALZE_FOR_PRINT = 63,
};

enum
{
	L0_POOL0_DDR_MU_DATA    = 0,
	L0_POOL1_DDR_HOST_DATA  = 1,
} L0Pool_e;

enum 
{
	L1_POOL0_MU_DATA_0,
	L1_POOL1_MU_DATA_1,
	L1_POOL1_TASK_INPUT = L1_POOL1_MU_DATA_1,
	L1_POOL2_TASK_OUTPUT,
	L1_POOL3_HOST_HEAP,
} L1Pool_e;

static constexpr uint64_t NUM_ADMIN_TASK_ID			= 32;
static constexpr uint64_t USER_TASK_ID_START		= NUM_ADMIN_TASK_ID + 1;

static constexpr uint64_t BIT_L0_CACHING			= 40ull;
static constexpr uint64_t BIT_L0_SKIP_ALLOCATE		= 44ull;
static constexpr uint64_t BIT_L0_SKIP_AGGR			= 45ull;
static constexpr uint64_t BIT_L1_SKIP_ALLOCATE		= 48ull;

static constexpr uint64_t MASK_L0_CACHING 			= (0x1ull << BIT_L0_CACHING);
static constexpr uint64_t MASK_L0_SKIP_ALLOCATE		= (0x1ull << BIT_L0_SKIP_ALLOCATE);
static constexpr uint64_t MASK_L0_SKIP_AGGR			= (0x1ull << BIT_L0_SKIP_AGGR);
static constexpr uint64_t MASK_L1_SKIP_ALLOCATE		= (0x1ull << BIT_L1_SKIP_ALLOCATE);

static constexpr uint64_t BIT_METIS_CORE_DDR		= 36ull;
static constexpr uint64_t BIT_METIS_CORE			= 32ull;

static constexpr uint64_t METIS_CORE_DDR_OFFSET	    = (0x1ull << BIT_METIS_CORE_DDR);
static constexpr uint64_t METIS_CORE_OFFSET			= (0x1ull << BIT_METIS_CORE);

#endif // _SYS_CONST_H_