#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

static constexpr uint64_t NUM_BITS_PER_CLST_CONFIG_BITMAP = 8;

//// CONFIG FOR MU CORE
// NUM OF OBJECT
static constexpr uint32_t NUM_THREAD_PER_MU             = 4;
static constexpr uint32_t NUM_CLUSTER_PER_ADMIN_MS      = 4;

// MAX NUM OF OBJECT
static constexpr uint32_t MAX_MTS_CORE                  = 2;
static constexpr uint32_t MAX_SUB_PER_CORE              = 4;
static constexpr uint32_t MAX_SUB_PER_DEVICE            = MAX_MTS_CORE * MAX_SUB_PER_CORE;
static constexpr uint64_t MAX_CLUSTER_PER_SUB           = 8;
static constexpr uint32_t MAX_ADMIN_PER_SUB             = MAX_CLUSTER_PER_SUB / NUM_CLUSTER_PER_ADMIN_MS;
static constexpr uint32_t MAX_CLUSTER_PER_CORE          = MAX_SUB_PER_CORE * MAX_CLUSTER_PER_SUB;
static constexpr uint32_t MAX_CLUSTER_PER_DEVICE        = MAX_SUB_PER_DEVICE * MAX_CLUSTER_PER_SUB;
static constexpr uint32_t MAX_MU_PER_CLUSTER            = 12;
static constexpr uint64_t MAX_THREAD_PER_CLUSTER        = MAX_MU_PER_CLUSTER * NUM_THREAD_PER_MU;

static constexpr uint32_t MAX_ADMIN_MS_PER_DEVICE       = 32;
static constexpr uint32_t MAX_MASTER_MS_PER_DEVICE      = 32;


// NUM of OBJECT for SIM
#if _SIM_
static constexpr uint64_t SIM_NUM_CORE                  = 1;
static constexpr uint64_t SIM_ADM_PER_SUB               = 1;
static constexpr uint64_t SIM_CLUSTER_BITMAP            = 0xF0Ful;
static constexpr uint32_t SIM_NUM_MU_PER_CLUSTER        = 8;
static constexpr uint32_t SIM_NUM_THREAD_PER_CLUSTER    = SIM_NUM_MU_PER_CLUSTER * NUM_THREAD_PER_MU;
#endif

//// CONFIG FOR MBOX
static constexpr uint64_t MBOX_SIZE_ADMIN = KB(128);
static constexpr uint64_t MBOX_SIZE_HMBOX = KB(16);
static constexpr uint64_t MBOX_SIZE_MTS   = KB(4);


static constexpr uint64_t TASK_OUTSTANDING_FACTOR = 4;
static constexpr uint64_t MAX_TASK_PER_ADMIN = NUM_CLUSTER_PER_ADMIN_MS * MAX_MU_PER_CLUSTER * NUM_THREAD_PER_MU * TASK_OUTSTANDING_FACTOR;

//// CONFIG FOR CACHE
// L0
static constexpr uint64_t BYTE_L0_CACHE_LINE    = 64;
static constexpr uint64_t NUM_L0_SET            = 1;
static constexpr uint64_t NUM_L0_WAY            = 8;
static constexpr uint64_t NUM_L0_POOL           = 2;
static constexpr uint64_t NUM_L0_REGION         = 2;
static constexpr uint64_t BYTE_L0_CACHE_SIZE    = BYTE_L0_CACHE_LINE * NUM_L0_SET * NUM_L0_WAY * NUM_L0_POOL;   // per mu thread

// L1
static constexpr uint64_t BYTE_L1_CACHE_LINE    = 64;
static constexpr uint64_t NUM_L1_SET            = 16;
static constexpr uint64_t NUM_L1_WAY            = 32;
static constexpr uint64_t NUM_L1_POOL           = 8;
static constexpr uint64_t NUM_L1_REGION         = 9;
static constexpr uint64_t BYTE_L1_CACHE_SIZE    = BYTE_L1_CACHE_LINE * NUM_L1_SET * NUM_L1_WAY * NUM_L1_POOL;

// L2
static constexpr uint64_t BYTE_L2_CACHE_LINE    = 256;
static constexpr uint64_t NUM_L2_SET            = 512;
static constexpr uint64_t NUM_L2_WAY            = 32;
static constexpr uint64_t NUM_L2_POOL           = 1;
static constexpr uint64_t NUM_L2_REGION         = 1;
static constexpr uint64_t BYTE_L2_CACHE_SIZE    = BYTE_L2_CACHE_LINE * NUM_L2_SET * NUM_L2_WAY * NUM_L2_POOL;

#define TASK_INPUT_L1S_CACHE_ON        (1)

#if _SIM_
    #define MBLAZE_DDR_TEXT_SECTION
    #define MBLAZE_DDR_DATA_SECTION
#else
    #define MBLAZE_DDR_TEXT_SECTION  //__attribute__ ((section(".ddrText")))
    #define MBLAZE_DDR_DATA_SECTION  //__attribute__ ((section(".ddrData")))
#endif

#endif // _SYS_CONFIG_H_
