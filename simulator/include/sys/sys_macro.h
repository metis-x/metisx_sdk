#ifndef _SYS_MACRO_H_
#define _SYS_MACRO_H_

#ifdef __cplusplus
template<typename T, uint64_t N>
constexpr uint64_t sizeofa(T(&)[N]) noexcept
{
	return N;
}

constexpr uint64_t log2(uint64_t n) noexcept
{
    return (n > 1) ? 1 + log2(n >> 1) : 0;
}
#else
#define sizeofa(x)  (sizeof(x) / sizeof((x)[0]))
#endif

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) (((uint64_t)x) << 30ULL)

#define BITMAP_FROM_BITPOS(bitpos) (1ULL<<(bitpos))
#define BIT_SET(mask, bitpos) ((mask) |= BITMAP_FROM_BITPOS(bitpos))
#define BIT_CLEAR(mask, bitpos) (mask &= ~BITMAP_FROM_BITPOS(bitpos))
#define BIT_FLIP(mask, bitpos) ((mask) ^= BITMAP_FROM_BITPOS(bitpos))
#define BIT_CHECK(mask, bitpos) (!!((mask) & BITMAP_FROM_BITPOS(bitpos)))        // '!!' to make sure this returns 0 or 1inline uint64_t BIT_COUNT(uint64_t mask)
inline uint64_t BIT_COUNT(uint64_t mask)
{
    uint64_t count = 0;

    while (mask)
    {
        count += (mask & 1);
        mask >>= 1;
    }

    return count;
}

#define GET_BIT_FIELD(mask, bitpos, size) (((mask) >> (bitpos)) & ((1ULL << (size))- 1 ))
#define LOWER32(value)          (uint32_t)(value & 0xFFFFFFFFul)
#define UPPER32(value)          (uint32_t)((value >> 32) & 0xFFFFFFFFul)
#define ALIGN_DOWN(base, size)	((base) & -((__typeof__ (base)) (size)))
#define ALIGN_UP(base, size)	ALIGN_DOWN ((base) + (size) - 1, (size))

#if (_DPIC_)
    #include "ctr_sub_dpic.h"
    #ifndef SET_REG
        #define SET_REG(addr, value) dpi_SET_REG64((uint64_t)(addr), (uint64_t)(value))
    #endif

    #ifndef SET_REG32
        #define SET_REG32(addr, value) dpi_SET_REG32((uint64_t)(addr), (uint32_t)(value))
    #endif

    #ifndef GET_REG
        #define GET_REG(addr) dpi_GET_REG64((uint64_t)(addr))
    #endif

    #ifndef GET_REG32
        #define GET_REG32(addr) (uint32_t)dpi_GET_REG32((uint64_t)(addr))
    #endif   

    #ifndef MEM_COPY
        #include <string.h>
        #define MEM_COPY(dst, src, size)                                      \
            for (uint32_t _idx = 0; _idx < (size) / sizeof(uint32_t); _idx++) \
            {                                                                 \
                SET_REG32(&(((uint32_t*)(dst))[_idx]), ((uint32_t*)(src))[_idx]);  \
            }
    #endif

    #define DPIC_DMB dpi_DMB()

#else
    #ifndef SET_REG
        #define SET_REG(addr, value) *((volatile uint64_t*)(addr)) = value
    #endif

    #ifndef SET_REG32
        #define SET_REG32(addr, value) *((volatile uint32_t*)(addr)) = (uint32_t)value
    #endif

    #ifndef GET_REG
        #define GET_REG(addr) *((volatile uint64_t*)(addr))
    #endif

    #ifndef GET_REG32
        #define GET_REG32(addr) *((volatile uint32_t*)(addr))
    #endif

    #ifndef MEM_COPY
        #include <string.h>
        #define MEM_COPY(dst, src, size) memcpy((void*)dst, (void*)src, size) // TODO - enable in micro blaze
    #endif
#endif

#ifndef REAL_MEM
#define REAL_MEM(addr) (addr)
#endif

#ifndef HW_MEM
#define HW_MEM(addr) (addr)
#endif

template<typename T>
inline T SET_ZERO_BASE(T value)
{
    return (value - (T)1);
}

template<typename T>
inline T GET_ZERO_BASE(T value)
{
    return (value + (T)1);
}

#endif // _SYS_MACRO_H_