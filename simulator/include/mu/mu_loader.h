#pragma once


typedef struct
{
    uint64_t startTimeStamp;
    uint64_t taskId;
    uint64_t inputBufAddr;
    uint64_t inputBufSize;
    uint64_t endTimeStamp;
}MuDebugInfo_t;


#ifndef _SIM_
    #define MASTER_MU_PARENT MasterMuParent



class MasterMuParent
{
public:
    void run();
};

    #define SLAVE_MU_PARENT MuSlaveParent

class MuSlaveParent
{
public:
#if (_SLAVE_)
    MUAllocator _muAllocator;
#endif
    void        run(uint64_t header);
};

    #define ADMIN_MU_PARENT AdminMuParent

class AdminMuParent
{
public:
    void run();
};
void copyDataSection(uint64_t* dst, uint64_t* dstEnd, uint64_t* src);
void setBssSection(uint64_t* dst, uint64_t* dstEnd);
void loader(uint64_t muType, bool readOnlyLoad);
void setStartDebugLog(MuHeader muHeader);
void setEndDebugLog(MuHeader muHeader);
void setMpu(MuHeader muHeader);
void invdCache(void);

    #define SLAVE_MU_START_UP                                              \
        extern "C" void _main()                                            \
        {                                                                  \
            uint64_t header;                                               \
            uint64_t type;                                                 \
            loader(MU_TYPE_SLAVE, true);                                   \
            while (1)                                                      \
            {                                                              \
                __launch(header);                                          \
                MuHeader muHeader(header);                                 \
                if (muHeader.muOpcode == MU_DEVICE_OPCODE_INVALID_L1C)     \
                {                                                          \
                    invdL1Pool(L1_POOL2_TASK_OUTPUT);                      \
                    invdL1Pool(L1_POOL3_HOST_HEAP);                        \
                    type = MU_DEVICE_OPCODE_INVALID_L1C;                   \
                }                                                          \
                else                                                       \
                {                                                          \
                    CLASS_NAME mu;                                         \
                    uint64_t startCycle, exec_startCycle;                  \
                    setMpu(muHeader);                                      \
                    setStartDebugLog(muHeader);                            \
                    loader(MU_TYPE_SLAVE, false);                          \
                    mu.MuAllocInit(header);                                \
                    mu.run(header);                                        \
                    type = MU_DEVICE_OPCODE_TERMINATION;                   \
                    invdCache();                                           \
                    setEndDebugLog(muHeader);                              \
                }                                                          \
                __terminate(header, type);                                 \
            }                                                              \
        }

    #define MASTER_MU_START_UP             \
        extern "C" void _main()            \
        {                                  \
            loader(MU_TYPE_MASTER, true);  \
            CLASS_NAME        mu;          \
            volatile uint64_t loop = 1ull; \
            while (loop)                   \
            {                              \
                mu.run();                  \
            }                              \
        }

    #define MASTER_MU_START_UP_ONE_TIME \
        extern "C" void _main()         \
        {                               \
            loader(MU_TYPE_MASTER, true);    \
            CLASS_NAME mu;              \
            mu.run();                   \
        }

    #define ADMIN_MU_START_UP              \
        extern "C" void _main()            \
        {                                  \
            loader(MU_TYPE_ADMIN, true);         \
            CLASS_NAME        mu;          \
            volatile uint64_t loop = 1ull; \
            while (loop)                   \
            {                              \
                mu.run();                  \
            }                              \
        }

    #define ADMIN_CONSTRUCTOR   CLASS_NAME()
    #define MASTER_CONSTRUCTOR  CLASS_NAME()
    #define SLAVE_CONSTRUCTOR   CLASS_NAME()
#else
typedef __int128          int128_t;
typedef unsigned __int128 uint128_t;
    #define MASTER_MU_PARENT \
    public                   \
        metisx::sim::MuCModel
    #define SLAVE_MU_PARENT \
    public                  \
        metisx::sim::SlaveMuCModel
    #define ADMIN_MU_PARENT \
    public                  \
        metisx::sim::MuCModel
    #define SLAVE_MU_START_UP
    #define MASTER_MU_START_UP
    #define MASTER_MU_START_UP_ONE_TIME
    #define ADMIN_MU_START_UP
    #define ADMIN_CONSTRUCTOR   \
        CLASS_NAME(uint64_t startAddr, uint64_t endAddr) : metisx::sim::MuCModel(startAddr, endAddr)
    #define MASTER_CONSTRUCTOR   \
        CLASS_NAME(uint64_t startAddr, uint64_t endAddr) : metisx::sim::MuCModel(startAddr, endAddr)
    #define SLAVE_CONSTRUCTOR   \
        CLASS_NAME(uint64_t startAddr, uint64_t endAddr) : metisx::sim::SlaveMuCModel(startAddr, endAddr)
#endif