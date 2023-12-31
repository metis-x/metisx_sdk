#pragma once

#define ADMIN_OUTSTANDING_CNT_ENABLE  (1)
#define MASTER_OUTSTANDING_CNT_ENABLE (0)
#define MU_DEBUG_STAT_ENABLE          (0)
static constexpr uint64_t DRAM_PER_THREAD((MB(4)));

enum MU_CSR
{
    MU_CSR_GPI_IN_BOUND  = 0,
    MU_CSR_GPI_OUT_BOUND = 1, // RSVD
    MU_CSR_MU_ID         = 2,
    MU_CSR_ICOUNT        = 3,
};

enum MU_DEVICE_OPCODE
{
    MU_DEVICE_OPCODE_NONE,
    MU_DEVICE_OPCODE_MESSAGE,
    MU_DEVICE_OPCODE_SYNC,
    MU_DEVICE_OPCODE_PRINT,
    MU_DEVICE_OPCODE_ALLOC_SHARED_MEM,
    MU_DEVICE_OPCODE_FREE_SHARED_MEM,
    MU_DEVICE_OPCODE_TERMINATION,
    MU_DEVICE_OPCODE_INVALID_L1C,
    MU_DEVICE_OPCODE_UPDATE_THREAD_BITMAP,
    MU_DEVICE_OPCODE_MAX_NUM = 0xFF
};

static constexpr uint64_t FIFO_INVALID_VALUE = UINT64_MAX;