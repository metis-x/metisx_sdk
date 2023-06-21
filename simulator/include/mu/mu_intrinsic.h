#pragma once

// fifop rd, rs1
#define __peek(__idx, __data)                   \
    asm volatile("fifop %0, %1"                 \
                 : "=r"(__data) /* Output */    \
                 : "r"(__idx) /* Input */);

// fifop rd, rs1
#define __pop(__idx, __data)                    \
    asm volatile("fifor %0, %1"                 \
                 : "=r"(__data) /* Output */    \
                 : "r"(__idx) /* Input */);

// fifop x0, rs1
#define __popOnly(__idx)          \
    asm volatile("fifor zero, %0" \
                 :                \
                 : "r"(__idx) /* Input */);

// lchr rd
#define __launch(__data)                        \
    asm volatile("lchr %0"                      \
                 : "=r"(__data) /* Output */);

// fifos rd, rs1, rs2
#define __push(__idx, __data, __result)             \
    asm volatile("fifos %0, %1, %2"                 \
                 : "=r"(__result) /* Output */      \
                 : "r"(__idx), "r"(__data)  /* Input */);

// cbusrd rd, rs1
#define __cread(__addr, __data)                 \
    asm volatile("cbusrd %0, %1"                \
                 : "=r"(__data) /* Output */    \
                 : "r"(__addr) /* Input */);

// cbuswr rs1, rs2
#define __cwrite(__addr, __data)                \
    asm volatile("cbuswr %0, %1"                \
                 :                              \
                 : "r"(__addr), "r"(__data) /* Input */);

//# (term rs2) / (term rs2, zero) both allowed,
//# strongly recommended to use (term t0)
//# CHECK: term t0
// term t0, zero
//# CHECK: term t0
// term t0
#define __terminate(__data, __type)                       \
    do                                                    \
    {                                                     \
        MuHeader __muHeader(__data);                      \
        __muHeader.muOpcode = __type;                       \
        __asm__ volatile(                                 \
            "sync %0\n\t"                                 \
            :                                             \
            : "r"(__muHeader.u64) /* Input */             \
        );                                                \
    } while (0);

//# CHECK: sync t0
// sync t0, zero
//# CHECK: sync t
// sync t0
#define __waitCsr(__addr, __mask) \
    asm volatile("csrwait %0, %1" \
                 : /* Output */   \
                 : "r"(__mask), "i"(__addr) /* Input */);

#define __sync_common(__inputData, __OutputData, __type) \
    do                                                   \
    {                                                    \
        MuHeader __muHeaderForSync(__inputData);         \
        __muHeaderForSync.muOpcode = __type;               \
        __asm__ volatile(                                \
            "sync %1\n\t"                                \
            "lchr %0"                                    \
            : "=r"(__OutputData)                         \
            : "r"(__muHeaderForSync.u64));               \
    } while (0);

#define __sync(__inputData, __OutputData)                                \
    do                                                                   \
    {                                                                    \
        __sync_common(__inputData, __OutputData, MU_DEVICE_OPCODE_SYNC); \
    } while (0);

#define __sync_print()                                                   \
    do                                                                   \
    {                                                                    \
        uint64_t __inputData;                                            \
        __getCsr(MU_CSR_MU_ID, __inputData);                             \
        __sync_common(__inputData, __inputData, MU_DEVICE_OPCODE_PRINT); \
    } while (0);

//# CHECK-INST:  csrget t0, 3
//# CHECK-ALIAS: csrget t0, 3
// csrget t0, 3
#define __getCsr(__addr, __data)             \
    asm volatile("csrget %0, %1"             \
                 : "=r"(__data) /* Output */ \
                 : "i"(__addr) /* Input */);

//# CHECK-INST:  csrset t1, 12
//# CHECK-ALIAS: csrset t1, 12
// csrset t1, 12
#define __orCsr(__addr, __bitpos)                             \
    do                                                        \
    {                                                         \
        uint64_t __mask = 1 << __bitpos;                      \
        asm volatile("csrset %0, %1"                          \
                     : /* Output */                           \
                     : "r"(__mask), "i"(__addr) /* Input */); \
    } while (0);

//# CHECK-INST:  csrclr t1, 1
//# CHECK-ALIAS: csrclr t1, 1
// csrclr t1, 1
#define __clrCsr(__addr, __bitpos)                            \
    do                                                        \
    {                                                         \
        uint64_t __mask = 1 << __bitpos;                      \
        asm volatile("csrclr %0, %1"                          \
                     : /* Output */                           \
                     : "r"(__mask), "i"(__addr) /* Input */); \
    } while (0);

/* cbusrd at CTR_SUB1_CTR_CTRL(0x00_C108_0000) + PMON_CYCLE_CNT(0x00210)*/
#define __getCycle(__data)              \
    do                                  \
    {                                   \
        __cread(0xC1080210ull, __data)  \
    } while (0);

//# CHECK-INST:  csrget  a1, 3073
//# CHECK-ALIAS: rdinstret a1
// rdinstret a1
#define __getInstructionCount(__data)    \
    do                                   \
    {                                    \
        __getCsr(MU_CSR_ICOUNT, __data); \
    } while (0);

//# CHECK: ctz32 t0, a0
// ctz32 t0, a0
#define __countTailingZero(__value, __data)  \
    asm volatile("ctz %0, %1"                \
                 : "=r"(__data) /* Output */ \
                 : "r"(__value) /* Input */);

//# CHECK: popc t0, a1
// popc t0, a1//
#define __popCount(__value, __data)          \
    asm volatile("popc %0, %1"               \
                 : "=r"(__data) /* Output */ \
                 : "r"(__value) /* Input */);

//# CHECK: pobit t0, a1, 100, 3
// pobit t0, a1, 100, 3
//# CHECK: pobit t0, a1, 3, 1
// pobit t0, a1, 3, 1//
#define __patternOccurenceBitmap(__value, __pattern, __bitType, __bitmap) \
    asm volatile("pobit %0, %1, %2, %3"                                   \
                 : "=r"(__bitmap) /* Output */                            \
                 : "r"(__value), "i"(__pattern), "i"(__bitType) /* Input */);

//# CHECK: rev t0, a1, 3
// rev t0, a1, 3
//# CHECK: rev t0, a1, 0
// rev t0, a1, 0
// compareBitType means  1 => 2bit, 2 => 4bit,  3 => 8bit
#define __reversePattern(__value, __bitType, __bitmap) \
    asm volatile("rev %0, %1, %2"                      \
                 : "=r"(__bitmap) /* Output */         \
                 : "r"(__value), "i"(__bitType) /* Input */);

#define __cop1(v0, v1, result)               \
    asm volatile("cop1 %0, %1, %2"           \
                 : "=r"(result) /* Output */ \
                 : "r"(v0), "r"(v1) /* Input */);


#define __cop2(v0, v1, result)               \
    asm volatile("cop1 %0, %1, %2"           \
                 : "=r"(result) /* Output */ \
                 : "r"(v0), "r"(v1) /* Input */);

#define __nop() \
    asm volatile("nop");

#define __dmb() \
    asm volatile("dmb");

#define __debugBreak() \
    asm volatile("dbreak");

#define __assert(__cond)        \
    if (!(__cond))              \
    {                           \
        asm volatile("dbreak"); \
    }

#define __invdL0(__pool)    \
    asm volatile("cinv %0"  \
                 :          \
                 : "i"(__pool) /* Input */);

#define __flushL0(__pool)   \
    asm volatile("cfls %0"  \
                 :          \
                 : "i"(__pool) /* Input */);

#define __mcpy(__v0, __v1, __result)                 \
        asm volatile("mcpy %0, %1, %2"/*mcpy*/           \
                 : "=r"(__result) /* Output */    \
                 : "r"(__v0), "r"(__v1) /* Input */);

#define __mset(__v0, __v1, __result)                  \
        asm volatile("mset %0, %1, %2" /*mset*/          \
                 : "=r"(__result) /* Output */    \
                 : "r"(__v0), "r"(__v1) /* Input */);

#define __dpes(__v0, __v1, __result)                   \
        asm volatile("dpes %0, %1, %2"           \
                 : "=r"(__result) /* Output */     \
                 : "r"(__v0), "r"(__v1) /* Input */);


#define __dper(__v0, __v1, __data)               \
    asm volatile("dper %0, %1, %2"            \
                 : "=r"(__data) /* Output */ \
                 : "r"(__v0), "r"(__v1) /* Input */);


