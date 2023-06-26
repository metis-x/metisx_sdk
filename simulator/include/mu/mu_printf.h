///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2019, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Tiny printf, sprintf and snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources.
//        Use this instead of bloated standard/newlib printf.
//        These routines are thread safe and reentrant.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PRINTF_H_
#define _PRINTF_H_
#include <stdarg.h>

#if (_SLAVE_)
//#define PRINTF_FTOA_BUFFER_SIZE
//#define PRINTF_DISABLE_SUPPORT_FLOAT
//#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#else
//#define PRINTF_FTOA_BUFFER_SIZE
#define PRINTF_DISABLE_SUPPORT_LONG_LONG
#define PRINTF_DISABLE_SUPPORT_FLOAT
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#endif
#define PRINTF_DISABLE_SUPPORT_PTRDIFF_T

#if _SIM_
#include <cstring>
#else
    #if _SLAVE_
        #define printf(...) \
            do  {                                               \
                printf_(__VA_ARGS__);                           \
                flushL0Pool(L0_POOL0_DDR_MU_DATA);              \
                invdL1Pool(L1_POOL0_MU_DATA_0);                 \
                invdL1Pool(L1_POOL1_MU_DATA_1);                 \
                invdL1Pool(L1_POOL2_MU_DATA_2);                 \
                invdL1Pool(L1_POOL3_MU_DATA_3);                 \
                invdL1Pool(L1_POOL4_MU_DATA_4);                 \
                __sync_print();                                 \
            } while(0);
    #elif (_MASTER_ || _ADMIN_)
        #define printf(...)                                                          \
            do                                                                       \
            {                                                                        \
                printf_(__VA_ARGS__);                                                \
                MuHeader __muHeaderForDebug;                                         \
                uint64_t __ret;                                                      \
                __getCsr(MU_CSR_MU_ID, __muHeaderForDebug.u64);                      \
                __push(MASTER_MS_QID_BLAZE_FOR_MS_PRINT, __muHeaderForDebug.u64, __ret); \
                __waitCsr(MU_CSR_GPI_IN_BOUND, BITMAP_FROM_BITPOS(MASTER_MS_QID_BLAZE_FOR_MS_PRINT));     \
                __pop(MASTER_MS_QID_BLAZE_FOR_MS_PRINT, __muHeaderForDebug.u64);        \
            } while (0);
    #else
        #define printf printf_
    #endif
#endif

int printf_(const char* format, ...);
int sprintf_(char* buffer, const char* format, ...);

#endif  // _PRINTF_H_
