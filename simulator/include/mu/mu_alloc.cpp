#include "sys.h"
#include <string.h>
#include "mu_const.h"
#ifdef _SIM_
    #include "sim.h"
#else
    #include "mu_intrinsic.h"
#endif
#include "mu_macro.h"

#include "mu_alloc.h"
#include "mu_printf.h"

#define likely(X)   __builtin_expect(!!(X), 1)
#define unlikely(X) __builtin_expect(!!(X), 0)

#if defined(_SLAVE_) || (_SIM_ == 1)

    #if (APPLY_NEW_MU_ALLOCATOR == 0)
        #define MU_ALLOC_DEBUG_LEVEL (0)

        #if (MU_ALLOC_DEBUG_LEVEL == 0)
            #define DEBUG_PRINTF_L1(...)
            #define DEBUG_PRINTF_L2(...)
        #elif (MU_ALLOC_DEBUG_LEVEL == 1)
            #define DEBUG_PRINTF_L1 printf
            #define DEBUG_PRINTF_L2(...)
        #elif (MU_ALLOC_DEBUG_LEVEL >= 2)
            #define DEBUG_PRINTF_L1 printf
            #define DEBUG_PRINTF_L2 printf
        #endif

        #if (MU_ALLOC_USE_HISTORY == 1)
            #define HISTORY_BUF_BITS            (8) // 8 for byte, actual width is 2^6 = 64
            #define setHistoryBufIdx(addr, idx) ((addr) | ((idx) << (64 - HISTORY_BUF_BITS)))
            #define getHistoryBufIdx(addr)      ((addr) >> (64 - HISTORY_BUF_BITS))
            #define stripHistoryBufIdx(addr)    (((addr) << HISTORY_BUF_BITS) >> HISTORY_BUF_BITS)

            #if (MU_ALLOC_USE_HISTORY_STATIC == 1)
static uint64_t _historyBuf[MU_ALLOC_HISTORY_BUF_MAX];
            #endif

static_assert(MU_ALLOC_HISTORY_BUF_MAX <= (1UL << HISTORY_BUF_BITS));
        #endif

void MUAllocator::initMUAllocator(uint64_t inHeader)
{
        #ifdef _SIM_
    // sim mu code is difficult to use mu remap addr
    // ex) array[x] = y;
    MuHeader header(inHeader);
    MuId_t   muId;
    muId.u16 = header.muId;
    // temp address
    _startAddress = (uint64_t)REAL_MEM(GET_REMAP_ADDR(MEM_START(MU_DRAM_PRINT_BUF_V_ADDR) + 0x10000, muId));
        #else
    extern uint64_t _dramMallocStart;
    _startAddress = (uint64_t)&_dramMallocStart;
        #endif

    reset();

        #if (MU_ALLOC_STATISTICS == 1)
    _maxAddress     = 0;
    _totalAllocated = 0;
        #endif
}

void MUAllocator::reset(void)
{
        #ifdef _SIM_
    _currAddress = _startAddress;
    _endAddress  = _startAddress + MEM_SIZE(MU_DRAM_V_ADDR);
        #else
    _currAddress  = _startAddress;
    _endAddress   = MEM_END(MU_DRAM_V_ADDR);
        #endif

    _allocCount  = 0;
    _lastAddress = _currAddress;

        #if (MU_ALLOC_USE_HISTORY == 1)
    _historyBitmap      = 0;
    _validBitmap        = 0;
    _historyBufAllocIdx = 0;
        #endif

    _isSharedMemAlloc = false;
}

void MUAllocator::historyBlockFree(uint64_t adjFreeCnt)
{
        #if (MU_ALLOC_USE_HISTORY == 1)
    _historyBufAllocIdx = (MU_ALLOC_HISTORY_BUF_MAX + _historyBufAllocIdx - adjFreeCnt) % MU_ALLOC_HISTORY_BUF_MAX;
    _currAddress        = stripHistoryBufIdx(_historyBuf[_historyBufAllocIdx]);
    DEBUG_PRINTF_L1("------ %d BLOCK FREE addr %x slot %d\n", adjFreeCnt, _currAddress, _historyBufAllocIdx);
    DEBUG_PRINTF_L2("vBitmap %08X_%08X\n", (uint32_t)(_validBitmap >> 32), (uint32_t)_validBitmap);
    if (adjFreeCnt == MU_ALLOC_HISTORY_BUF_MAX)
    {
        /* According to the C standard, the result is undefined
        if the shift amount is negative, or greater than or equal
        to the number of bits in the variable type.
        And the shift amount is masked on most platforms. */
        /* Thus, (uint64_t >> 64 == uint64_t >> 0) */
        _historyBitmap = 0;
        _validBitmap   = 0;
    }
    else
    {
        _historyBitmap = _historyBitmap >> adjFreeCnt;
        _validBitmap   = _validBitmap >> adjFreeCnt;
    }
    DEBUG_PRINTF_L2("vBitmap %08X_%08X (after)\n", (uint32_t)(_validBitmap >> 32), (uint32_t)_validBitmap);
    DEBUG_PRINTF_L2("hBitmap %08X_%08X (after)\n", (uint32_t)(_historyBitmap >> 32), (uint32_t)_historyBitmap);
        #endif
}

uint64_t alignMuAllocUnitSize(uint64_t size)
{
    const uint64_t muAllocUnitSize = MU_ALLOC_UNIT_SIZE;
    if ((size % muAllocUnitSize != 0) || size == 0)
    {
        uint64_t MuAllocUnitSizeBit = log2(muAllocUnitSize);
        size                        = ((size >> MuAllocUnitSizeBit) + 1) << MuAllocUnitSizeBit;
    }
    return size;
}

void* MUAllocator::alloc(uint64_t size, uint64_t alignBit)
{
    size = alignMuAllocUnitSize(size);

    if ((alignUp(_currAddress, alignBit) + size) >= _endAddress)
        return nullptr;

    _lastAddress = alignUp(_currAddress, alignBit);
    _currAddress = _lastAddress + size;
        #if (MU_ALLOC_USE_HISTORY == 1)
    void* ret = (void*)setHistoryBufIdx(_lastAddress, _historyBufAllocIdx);
        #else
    void* ret     = (void*)_lastAddress;
        #endif

    DEBUG_PRINTF_L2("alloc addr %x size %d\n", _lastAddress, size);

    if (likely(_isSharedMemAlloc == false))
    {
        #if (MU_ALLOC_USE_HISTORY == 1)
        _historyBitmap                   = (_historyBitmap << 1) + 1;
        _validBitmap                     = (_validBitmap << 1) + 1;
        _historyBuf[_historyBufAllocIdx] = (uint64_t)ret;
        DEBUG_PRINTF_L2(" --- history alloc slot %d addr %x%x\n", _historyBufAllocIdx, ((uint64_t)ret) >> 32, (uint64_t)ret);
        DEBUG_PRINTF_L2("bitmap %08X_%08X\n", (uint32_t)(_historyBitmap >> 32), (uint32_t)_historyBitmap);
        _historyBufAllocIdx = (_historyBufAllocIdx + 1) % MU_ALLOC_HISTORY_BUF_MAX;
        #endif

        _allocCount++;
    }

        #if (MU_ALLOC_STATISTICS == 1)
    _totalAllocated += size;
    if (_maxAddress < _currAddress)
        _maxAddress = _currAddress;
        #endif

    return ret;
}

void* MUAllocator::calloc(uint64_t cnt, uint64_t size, uint64_t alignBit)
{
    void* ret = alloc(cnt * size, alignBit);
    if (likely(ret != nullptr))
        memset(ret, 0, cnt * size);
    return ret;
}

void* MUAllocator::realloc(void* src, uint64_t src_size, uint64_t dst_size, uint64_t alignBit)
{
    uint64_t srcAddress = (uint64_t)src;
        #if (MU_ALLOC_USE_HISTORY == 1)
    srcAddress = stripHistoryBufIdx(srcAddress);
        #endif
    uint64_t alignSrcSize = alignMuAllocUnitSize(src_size);
    if (srcAddress >= MEM_START(DDR_TASK_INPUT) || alignSrcSize < dst_size)
    {
        // malloc condition
        // 1. src is not MU_DRAM_V area (e.g., input buffer)
        // 2. src_size < dst_size

        void* ret = alloc(dst_size, alignBit);
        if (likely(ret != nullptr && src))
        {
            if (src_size == UNKNOWN_SRC_SIZE)
            {
                memcpy(ret, src, dst_size);
            }
            else
            {
                memcpy(ret, src, src_size);
            }

            if (srcAddress < MEM_START(DDR_TASK_INPUT))
            {
                free(src);
            }
        }
        return ret;
    }

    if (src == nullptr)
    {
        return alloc(dst_size, alignBit);
    }
    else
    {
        // don't need to realloc()
        return src;
    }
}

void* MUAllocator::free(void* src)
{
    uint64_t freeAddr = (uint64_t)src;

    if (likely(_isSharedMemAlloc == false))
    {
        #if (MU_ALLOC_USE_HISTORY == 1)
        uint64_t hBufSlot = getHistoryBufIdx(freeAddr);
        if (_historyBuf[hBufSlot] == freeAddr)
        {
            uint64_t bitmapIdx = (MU_ALLOC_HISTORY_BUF_MAX + _historyBufAllocIdx - hBufSlot - 1) % MU_ALLOC_HISTORY_BUF_MAX;
            DEBUG_PRINTF_L2(" --- free hBufSlot %d addr %x, bitmapIdx %d\n", hBufSlot, (uint64_t)src, bitmapIdx);

            #if (_SIM_ == 0)
            __assert(BIT_CHECK(_historyBitmap, bitmapIdx));
            #endif
            BIT_CLEAR(_historyBitmap, bitmapIdx);

            DEBUG_PRINTF_L2("bitmap %08X_%08X\n", (uint32_t)(_historyBitmap >> 32), (uint32_t)_historyBitmap);

            if (bitmapIdx == 0)
            {
                uint64_t adjFreeCnt;
                uint64_t hBitmap = ((uint64_t)_historyBitmap) | (~((uint64_t)_validBitmap));
                __countTailingZero(hBitmap, adjFreeCnt);
                if (adjFreeCnt < MU_ALLOC_HISTORY_THRESHOLD)
                    adjFreeCnt = 1;
                historyBlockFree(adjFreeCnt);
            }
        }
        freeAddr = stripHistoryBufIdx(freeAddr);
        #else // (MU_ALLOC_USE_HISTORY == 1)
        if (_lastAddress == freeAddr)
            _currAddress = _lastAddress;

        DEBUG_PRINTF_L2("free addr %x, currAddr %x, shrinked %d\n", src, _currAddress, (uint64_t)src == _lastAddress);
        #endif

        if (likely(freeAddr >= _startAddress && freeAddr <= _endAddress))
        {
            _allocCount--;
            if (unlikely(_allocCount == 0))
            {
                reset();
            }
        }
    }
    else
    {
        #if (MU_ALLOC_USE_HISTORY == 1)
        freeAddr = stripHistoryBufIdx(freeAddr);
        #endif

        if (_lastAddress == freeAddr)
            _currAddress = _lastAddress;
    }

    return src;
}

void MUAllocator::setSharedMem(MuHeader muHeader)
{
    uint64_t allocIdx = muHeader.taskId;

    uint64_t vClusterId = GET_REG32(MEM_START(DDR_CTRL_INFO) + offsetof(DdrCtrlInfo_t, virtualClusterIdList) + sizeof(uint32_t) * (muHeader.clusterId + muHeader.subId * MAX_CLUSTER_PER_SUB));

    _currAddress = (uint64_t)REAL_MEM(
        MEM_START(DDR_SLAVE_MEM_CLST0_SHARED)
        + MEM_SIZE(DDR_SLAVE_MEM_CLST0) * (vClusterId)
        + SharedMemAllocator::ALLOC_MEM_SIZE * allocIdx);
    _endAddress = _currAddress + SharedMemAllocator::ALLOC_MEM_SIZE;

    DEBUG_PRINTF_L1("------- no local >> alloc shared -------\n");
    _isSharedMemAlloc = true;

        #if (MU_ALLOC_USE_HISTORY == 1)
    _historyBitmap      = 0;
    _validBitmap        = 0;
    _historyBufAllocIdx = 0;
        #endif
}

bool MUAllocator::isSharedMemAlloc(void)
{
    return _isSharedMemAlloc;
}

        #if (MU_ALLOC_STATISTICS == 1)
uint64_t MUAllocator::getCurrAddr(void)
{
    return _currAddress;
}

uint64_t MUAllocator::getMaxAddr(void)
{
    return _maxAddress;
}

uint64_t MUAllocator::getTotalAlloc(void)
{
    return _totalAllocated;
}
        #endif

    #else

void MUAllocator::initMUAllocator(uint64_t inHeader)
{

        #ifdef _SIM_
    // sim mu code is difficult to use mu remap addr
    // ex) array[x] = y;
    MuHeader header(inHeader);
    MuId_t   muId;
    muId.u16 = header.muId;
    // temp address
    startAddress_ = (uint64_t)REAL_MEM(GET_REMAP_ADDR(MEM_START(MU_DRAM_PRINT_BUF_V_ADDR) + 0x10000, muId));
    endAddress_   = startAddress_ + MEM_SIZE(MU_DRAM_V_ADDR);
        #else
    extern uint64_t _dramMallocStart;
    startAddress_ = (uint64_t)&_dramMallocStart;
    startAddress_ = ALIGN_UP(startAddress_, 1024);
    endAddress_   = MEM_END(MU_DRAM_V_ADDR) + 1;
        #endif

    init(startAddress_, endAddress_);

    reset();
}

void MUAllocator::setSharedMem(MuHeader muHeader)
{

    uint64_t allocIdx = muHeader.taskId;

    uint64_t vClusterId = GET_REG32(MEM_START(DDR_CTRL_INFO) + offsetof(DdrCtrlInfo_t, virtualClusterIdList) + sizeof(uint32_t) * (muHeader.clusterId + muHeader.subId * MAX_CLUSTER_PER_SUB));

    uint64_t startAddress = (uint64_t)REAL_MEM(
        MEM_START(DDR_SLAVE_MEM_CLST0_SHARED)
        + MEM_SIZE(DDR_SLAVE_MEM_CLST0) * (vClusterId)
        + SharedMemAllocator::ALLOC_MEM_SIZE * allocIdx);
    uint64_t endAddress = startAddress + SharedMemAllocator::ALLOC_MEM_SIZE;
  
    slobAllocator.initExternal(startAddress, endAddress);

    allocSharedMem_ = true;
}

void* MUAllocator::calloc(uint64_t count, uint64_t size, uint64_t alignBit)
{
    void* ret = alloc(count * size, alignBit);
    if (ret != nullptr)
        memset(ret, 0, count * size);
    return ret;
}

void* MUAllocator::realloc(void* src, uint64_t src_size, uint64_t dst_size, uint64_t alignBit)
{
    uint64_t curPoolBufferSize = 0;
    uint64_t srcAddress        = reinterpret_cast<uint64_t>(src);

    curPoolBufferSize = poolSize(src);

    if (curPoolBufferSize == 0)
    {
        curPoolBufferSize = src_size;
    }

    if (likely(src != nullptr))
    {
        if (srcAddress >= MEM_START(DDR_TASK_INPUT) || curPoolBufferSize < dst_size)
        {
            void* ret = alloc(dst_size, alignBit);
            if (likely(ret != nullptr))
            {
                if (likely(src_size != UNKNOWN_SRC_SIZE))
                {
                    memcpy(ret, src, src_size);
                }
                else
                {
                    memcpy(ret, src, dst_size);
                }
            }
            free(src);
            return ret;
        }
        else
        {
            return src;
        }
    }
    else
    {
        return alloc(dst_size, alignBit);
    }

    return nullptr;
}


void* SlobAllocator::alloc(uint64_t reqSize, bool isExternal)
{

    reqSize = ALIGN_UP(reqSize, KB(1));
    SlobChunkInfo_t* curFreeChunk;

    if (isExternal == false)
    {
        curFreeChunk = freeChunkHeader_;
    }
    else
    {
        curFreeChunk = externalFreeChunkHeader_;
    }

    while (curFreeChunk != nullptr)
    {
        uint64_t freeChunkSize = curFreeChunk->chunkSize;
        if (reqSize == freeChunkSize)
        {
            uint64_t allocatedAddress;
            removeFreeChunk(curFreeChunk);
            allocatedAddress = reinterpret_cast<uint64_t>(curFreeChunk) + sizeof(SlobChunkInfo_t);
            curFreeChunk->allocStatus = AllocStatus::Allocated;
            
            return  reinterpret_cast<void*>(allocatedAddress);
        }
        else if (reqSize + sizeof(SlobChunkInfo_t) < freeChunkSize)
        {
            uint64_t allocatedAddress;
            removeFreeChunk(curFreeChunk);
            allocatedAddress = reinterpret_cast<uint64_t>(curFreeChunk) + freeChunkSize + sizeof(SlobChunkInfo_t); 
            allocatedAddress -= reqSize;
            
            SlobChunkInfo_t* allocatedChunk = reinterpret_cast<SlobChunkInfo_t*>((allocatedAddress - sizeof(SlobChunkInfo_t)));
            allocatedChunk->nextChunk = curFreeChunk->nextChunk;
            allocatedChunk->prevChunk = reinterpret_cast<void*>(curFreeChunk);
            allocatedChunk->nextFreeChunk = nullptr;
            allocatedChunk->prevFreeChunk = nullptr;
            allocatedChunk->chunkSize = reqSize;
            allocatedChunk->tag = Tag;
            allocatedChunk->allocStatus = AllocStatus::Allocated;

                
            if (curFreeChunk->nextChunk != nullptr)
            {
                reinterpret_cast<SlobChunkInfo_t*>(curFreeChunk->nextChunk)->prevChunk = reinterpret_cast<void*>(allocatedChunk);
            }

            curFreeChunk->chunkSize = freeChunkSize - (reqSize + sizeof(SlobChunkInfo_t));
            curFreeChunk->nextChunk = reinterpret_cast<void*>(allocatedChunk);
                
            insertFreeChunk(curFreeChunk);
            return reinterpret_cast<void*>(allocatedAddress);
        }
        
        curFreeChunk     = reinterpret_cast<SlobChunkInfo_t*>(curFreeChunk->nextFreeChunk);
    } 

    if (isExternal == false && externalTotalCapacity_ != 0)
    {
        return alloc(reqSize, true);
    }
    else
    {
        return nullptr;
    }
}

void SlobAllocator::removeFreeChunk(SlobChunkInfo_t* curr)
{
    if (freeChunkHeader_ == curr)
    {
        freeChunkHeader_ = reinterpret_cast<SlobChunkInfo_t*>(curr->nextFreeChunk);
    }

    if (curr->prevFreeChunk != nullptr)
    {
        reinterpret_cast<SlobChunkInfo_t*>(curr->prevFreeChunk)->nextFreeChunk = curr->nextFreeChunk;
    }

    if (curr->nextFreeChunk != nullptr)
    {
        reinterpret_cast<SlobChunkInfo_t*>(curr->nextFreeChunk)->prevFreeChunk = curr->prevFreeChunk;
    }
    curr->nextFreeChunk = nullptr;
    curr->prevFreeChunk = nullptr;
}

void SlobAllocator::insertFreeChunk(SlobChunkInfo_t* newFreeChunk)
{
    if (freeChunkHeader_ == nullptr)
    {
        freeChunkHeader_ = newFreeChunk;
        newFreeChunk->nextFreeChunk = nullptr;
        newFreeChunk->prevFreeChunk = nullptr;
        return;
    }

    SlobChunkInfo_t* curr = freeChunkHeader_;
    SlobChunkInfo_t* last;
    do
    {
        if (newFreeChunk->chunkSize <= curr->chunkSize)
        {
            
            if (curr->prevFreeChunk != nullptr)
            {
                reinterpret_cast<SlobChunkInfo_t*>(curr->prevFreeChunk)->nextFreeChunk = reinterpret_cast<void*>(newFreeChunk);
            }
            else
            {
                freeChunkHeader_ = newFreeChunk;
            }
            
            newFreeChunk->prevFreeChunk = curr->prevFreeChunk;
            newFreeChunk->nextFreeChunk = reinterpret_cast<void*>(curr);
            curr->prevFreeChunk = reinterpret_cast<void*>(newFreeChunk);
            return;
        }
        last = curr;
        curr = reinterpret_cast<SlobChunkInfo_t*>(curr->nextFreeChunk);
    } while (curr != nullptr);

    last->nextFreeChunk = reinterpret_cast<void*>(newFreeChunk);
    newFreeChunk->prevFreeChunk = reinterpret_cast<void*>(last);
    newFreeChunk->nextFreeChunk = nullptr;
}

// merge src chunk into dst chunk
void SlobAllocator::mergeChunk(SlobChunkInfo_t* dst, SlobChunkInfo_t* src)
{
    removeFreeChunk(dst);
    removeFreeChunk(src);

    dst->chunkSize += (src->chunkSize + sizeof(SlobChunkInfo_t));
    dst->nextChunk = src->nextChunk;
    
    if (src->nextChunk != nullptr)
    {
        reinterpret_cast<SlobChunkInfo_t*>(src->nextChunk)->prevChunk = reinterpret_cast<void*>(dst);
    }
    insertFreeChunk(dst);
}


void* SlobAllocator::free(void* src)
{
    uint64_t srcAddress = reinterpret_cast<uint64_t>(src);
    SlobChunkInfo_t* curChunk = reinterpret_cast<SlobChunkInfo_t*>(srcAddress - sizeof(SlobChunkInfo_t));
    SlobChunkInfo_t* nextChunk = reinterpret_cast<SlobChunkInfo_t*>(curChunk->nextChunk);
    SlobChunkInfo_t* prevChunk = reinterpret_cast<SlobChunkInfo_t*>(curChunk->prevChunk);
    
    if (curChunk->tag != Tag)
    {
        #if _SIM_ == 0
        __assert(0);
        #endif
    }

    curChunk->allocStatus = AllocStatus::UnAllocated;
    insertFreeChunk(curChunk);
    if (nextChunk != nullptr && nextChunk->allocStatus == AllocStatus::UnAllocated)
    {
        mergeChunk(curChunk, nextChunk);
    }
    if (prevChunk != nullptr && prevChunk->allocStatus == AllocStatus::UnAllocated)
    {
        mergeChunk(prevChunk, curChunk);
    }
    return src;
}

    #endif // NEW ALLOCATOR
#endif     // _SIM_ && _SLAVE_

SharedMemAllocator::SharedMemAllocator()
{
    _freeBitmap = (1ull << NUM_ALLOC_MEM) - 1;
    for (uint64_t idx = 0; idx < MAX_THREAD_PER_CLUSTER; idx++)
    {
        _allocBitmap[idx] = 0;
    }
}

SharedMemAllocator::~SharedMemAllocator()
{
}

uint64_t SharedMemAllocator::alloc(uint64_t slaveId)
{
    uint64_t allocIdx;
    __countTailingZero(_freeBitmap, allocIdx);
    if (allocIdx == ALLOCATION_FAIL)
    {
// TODO
#if (_SIM_ == 0)
        __assert(0);
#endif
    }

    _freeBitmap &= ~(1ull << allocIdx);
    _allocBitmap[slaveId] |= (1ull << allocIdx);

    return allocIdx;
}

void SharedMemAllocator::freeThread(uint64_t threadId)
{
    _freeBitmap |= _allocBitmap[threadId];
    _allocBitmap[threadId] = 0;
}

void SharedMemAllocator::free(uint64_t threadId, uint64_t idx)
{
    _freeBitmap |= (1ull << idx);
    _allocBitmap[threadId] &= ~(1ull << idx);
}