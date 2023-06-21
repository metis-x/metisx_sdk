#pragma once

#include <stdint.h>

#define APPLY_NEW_MU_ALLOCATOR (1)
#if _SIM_
    #include <assert.h>
    #define MU_ALLOC_ASSERT(cond) assert(cond);
#else
    #define MU_ALLOC_ASSERT(cond) __assert(cond);
#endif

#if defined(_SLAVE_) || (_SIM_ == 1)

    #if (APPLY_NEW_MU_ALLOCATOR == 0)

        #define MU_ALLOC_STATISTICS (0)

        #ifdef _SIM_
            #define MU_ALLOC_USE_HISTORY (0)
        #else
            #define MU_ALLOC_USE_HISTORY (1)
        #endif

        #if (MU_ALLOC_USE_HISTORY == 1)
            #define MU_ALLOC_USE_HISTORY_STATIC (0)
            #define MU_ALLOC_HISTORY_BUF_MAX    (64)
            #define MU_ALLOC_HISTORY_THRESHOLD  (MU_ALLOC_HISTORY_BUF_MAX / 2)

            #if (MU_ALLOC_HISTORY_BUF_MAX == 8)
typedef uint8_t allocHistoryBitmap_t;
            #elif (MU_ALLOC_HISTORY_BUF_MAX == 16)
typedef uint16_t allocHistoryBitmap_t;
            #elif (MU_ALLOC_HISTORY_BUF_MAX == 32)
typedef uint32_t allocHistoryBitmap_t;
            #elif (MU_ALLOC_HISTORY_BUF_MAX == 64)
typedef uint64_t allocHistoryBitmap_t;
            #else
                #error
            #endif
        #endif

class MUAllocator
{
    private:
        uint64_t _startAddress;
        uint64_t _currAddress;
        uint64_t _endAddress;
        int64_t  _allocCount;
        uint64_t _lastAddress;
        #if (MU_ALLOC_USE_HISTORY == 1)
        allocHistoryBitmap_t _historyBitmap;
        allocHistoryBitmap_t _validBitmap;
        uint64_t             _historyBufAllocIdx;
            #if (MU_ALLOC_USE_HISTORY_STATIC == 0)
        uint64_t _historyBuf[MU_ALLOC_HISTORY_BUF_MAX];
            #endif
        #endif
        bool _isSharedMemAlloc;

        #if (MU_ALLOC_STATISTICS == 1)
        uint64_t _maxAddress;
        uint64_t _totalAllocated;
        #endif

    private:
        inline uint64_t alignUp(uint64_t address, uint64_t alignBit)
        {
            return ALIGN_UP(address, (1 << alignBit));
        }

        inline bool snapshotExists(void);
        void        historyBlockFree(uint64_t adjFreeCnt);

    public:
        MUAllocator(uint64_t header = 0)
        {
            initMUAllocator(header);
        }

        ~MUAllocator()
        {
        }

        void snapshotRecord(void);

        void     initMUAllocator(uint64_t header);
        void     reset(void);
        void*    alloc(uint64_t size, uint64_t alignBit);
        void*    calloc(uint64_t cnt, uint64_t size, uint64_t alignBit);
        void*    realloc(void* src, uint64_t src_size, uint64_t dst_size, uint64_t alignBit);
        void*    free(void* src);
        uint64_t remap(uint64_t address);
        void     setSharedMem(MuHeader muHeader);
        bool     isSharedMemAlloc(void);

        #if (MU_ALLOC_STATISTICS == 1)
        uint64_t getCurrAddr(void);
        uint64_t getMaxAddr(void);
        uint64_t getTotalAlloc(void);
        #endif
};

    #else

template <uint64_t Size = 0, uint64_t EntryCount = 0>
class BitmapAllocator
{
    public:
        static constexpr uint64_t NumBitsPerBitmap = 64;
        static constexpr uint64_t BitmapSize       = (EntryCount % NumBitsPerBitmap == 0) ? (EntryCount / NumBitsPerBitmap) : (EntryCount / NumBitsPerBitmap) + 1;

        BitmapAllocator()
        {
        }

        ~BitmapAllocator()
        {
        }

        void init(uint64_t baseAddress, uint64_t endAddress)
        {
            baseAddress_ = baseAddress;
            endAddress_  = endAddress;
            log2Size_    = log2(Size);
            reset();
        }

        void reset(void)
        {
            freeCount_               = EntryCount;
            lastIndex_               = 0;
            lastBit_                 = 0;
            uint64_t totalEntryCount = EntryCount;
            for (uint64_t i = 0; i < BitmapSize; i++)
            {
                if (totalEntryCount >= NumBitsPerBitmap)
                {
                    bitmap_[i] = UINT64_MAX;
                }
                else if (totalEntryCount < NumBitsPerBitmap)
                {
                    bitmap_[i] = (0x1ull << totalEntryCount) - 0x1ull;
                    break;
                }
                totalEntryCount -= NumBitsPerBitmap;
            }
        }

        void* alloc(uint64_t requestSize)
        {
            if (requestSize > Size)
                return nullptr;

            if (freeCount_ == 0)
                return nullptr;

            uint64_t bitmapSize = BitmapSize;
            if (lastBit_ > 0)
                bitmapSize++;

            do
            {
                uint64_t curBitmap = bitmap_[lastIndex_];
                curBitmap >>= lastBit_;
                if (curBitmap != 0x0ull)
                {
                    uint64_t bit        = 0;
        #if _SIM_
                    uint64_t tempBitmap = curBitmap;
                    for (uint32_t i = 0; i < NumBitsPerBitmap; i++)
                    {
                        if (tempBitmap & 0x1)
                        {
                            bit = i;
                            break;
                        }
                        tempBitmap >>= 1;
                    }
        #else
                    __countTailingZero(curBitmap, bit);
        #endif
                    bit += lastBit_;

                    uint64_t allocIndex       = (lastIndex_ * NumBitsPerBitmap) + bit;
                    uint64_t allocatedAddress = baseAddress_ + (allocIndex << log2Size_);
                    bitmap_[lastIndex_] &= ~(0x1ull << bit);
                    lastBit_ = (bit + 1) % NumBitsPerBitmap;
                    if (lastBit_ == 0)
                    {
                        lastIndex_ = (lastIndex_ + 1) % BitmapSize;
                    }
                    freeCount_--;
                    return reinterpret_cast<void*>(allocatedAddress);
                }

                lastBit_   = 0;
                lastIndex_ = (lastIndex_ + 1) % BitmapSize;
                bitmapSize--;
            } while (bitmapSize != 0);

            MU_ALLOC_ASSERT(0);
            return nullptr;
        }

        void* free(void* src)
        {
            MU_ALLOC_ASSERT(isEntryOf(src));
            uint64_t srcAddress = reinterpret_cast<uint64_t>(src);
            srcAddress -= baseAddress_;
            uint64_t index       = srcAddress >> log2Size_;
            uint64_t bitmapIndex = index / NumBitsPerBitmap;
            uint64_t bitIndex    = index % NumBitsPerBitmap;

            lastBit_   = bitIndex;
            lastIndex_ = bitmapIndex;

            uint64_t bitCheck = (bitmap_[bitmapIndex] >> bitIndex) & 0x1ull;
            MU_ALLOC_ASSERT(bitCheck == 0);

            bitmap_[bitmapIndex] |= (0x1ull << bitIndex);
            freeCount_++;

            return src;
        }

        uint64_t size(void* src) const
        {
            return Size;
        }

        bool isEntryOf(void* src) const
        {
            uint64_t srcAddress = reinterpret_cast<uint64_t>(src);
            return (baseAddress_ <= srcAddress && srcAddress + Size <= endAddress_);
        }

        uint64_t totalCapacity(void) const
        {
            return EntryCount * Size;
        }

        uint64_t freeCount(void) const
        {
            return freeCount_;
        }

        uint32_t bitmapSize(void) const
        {
            return BitmapSize;
        }

        uint64_t getBitmap(uint64_t index) const
        {
            return bitmap_[index];
        }

    private:
        uint64_t bitmap_[BitmapSize];
        uint64_t baseAddress_;
        uint64_t endAddress_;
        uint32_t log2Size_;
        uint32_t freeCount_;
        uint64_t lastBit_;
        uint64_t lastIndex_;
};

class IncrementalAllocator
{
    private:
        typedef struct
        {
                uint64_t tag;
                uint64_t size;
        } IncrementalAllocatorMeta_t;

        static constexpr uint64_t TagValue     = 0xABCDABCDABCDABCDull;
        static constexpr uint32_t MaxRegionCnt = 4;

    public:
        IncrementalAllocator()
        {
        }

        ~IncrementalAllocator()
        {
        }

        void init(uint64_t startAddress, uint64_t endAddress)
        {
            startAddress_[0] = startAddress;
            endAddress_[0]   = endAddress;
        }

        void reset(void)
        {
            lastAddress_[0]    = startAddress_[0];
            currentAddress_[0] = startAddress_[0];
            regionCnt_         = 1;
        }

        void addRegion(uint64_t startAddress, uint64_t endAddress)
        {
            startAddress_[regionCnt_]   = startAddress;
            currentAddress_[regionCnt_] = startAddress;
            endAddress_[regionCnt_]     = endAddress;
            regionCnt_++;
            MU_ALLOC_ASSERT(regionCnt_ <= MaxRegionCnt);
        }

        void* alloc(uint64_t size)
        {
            size = ALIGN_UP(size, KB(4));

            for (uint32_t i = 0; i < regionCnt_; i++)
            {
                if (currentAddress_[i] + size + sizeof(IncrementalAllocatorMeta_t) < endAddress_[i])
                {
                    lastAddress_[i]                                      = currentAddress_[i];
                    currentAddress_[i]                                   = lastAddress_[i] + size + sizeof(IncrementalAllocatorMeta_t);
                    ((IncrementalAllocatorMeta_t*)lastAddress_[i])->size = size;
                    //((IncrementalAllocatorMeta_t*)lastAddress_[i])->tag  = TagValue;
                    return reinterpret_cast<void*>(lastAddress_[i] + sizeof(IncrementalAllocatorMeta_t));
                }
            }
            return nullptr;
        }

        void* free(void* src)
        {
            uint64_t srcAddress = reinterpret_cast<uint64_t>(src);
            srcAddress -= sizeof(IncrementalAllocatorMeta_t);
            // MU_ALLOC_ASSERT(((IncrementalAllocatorMeta_t*)srcAddress)->tag == TagValue);

            for (uint32_t i = 0; i < regionCnt_; i++)
            {
                if (srcAddress == lastAddress_[i])
                {
                    currentAddress_[i] = srcAddress;
                    break;
                }
            }
            return src;
        }

        bool isEntryOf(void* src) const
        {
            uint64_t srcAddress = reinterpret_cast<uint64_t>(src);
            for (uint32_t i = 0; i < regionCnt_; i++)
            {
                if (startAddress_[i] <= srcAddress && srcAddress <= endAddress_[i])
                {
                    return true;
                }
            }
            return false;
        }

        uint64_t size(void* src)
        {
            uint64_t metaAddress = reinterpret_cast<uint64_t>(src);
            metaAddress -= sizeof(IncrementalAllocatorMeta_t);
            // MU_ALLOC_ASSERT(((IncrementalAllocatorMeta_t*)metaAddress)->tag == TagValue);
            return ((IncrementalAllocatorMeta_t*)metaAddress)->size;
        }

    private:
        uint32_t regionCnt_;
        uint64_t startAddress_[MaxRegionCnt];
        uint64_t endAddress_[MaxRegionCnt];
        uint64_t lastAddress_[MaxRegionCnt];
        uint64_t currentAddress_[MaxRegionCnt];
};

class SlobAllocator
{
    private:
        enum class AllocStatus : uint32_t
        {
            UnAllocated,
            Allocated,
        };

        static const uint64_t Tag = 0xABCDABCDABCDABCDull;

        typedef struct
        {
                void*       nextChunk;
                void*       prevChunk;
                void*       nextFreeChunk;
                void*       prevFreeChunk;
                uint64_t    tag;
                AllocStatus allocStatus;
                uint32_t    chunkSize;
        } SlobChunkInfo_t;

    public:
        SlobAllocator()
        {
        }

        ~SlobAllocator()
        {
        }

        void init(uint64_t startAddr, uint64_t endAddr)
        {
            startAddress_  = startAddr;
            endAddress_    = endAddr;
            totalCapacity_ = ((endAddress_ - startAddress_) - sizeof(SlobChunkInfo_t));
        }

        void reset(void)
        {
            freeChunkHeader_                = reinterpret_cast<SlobChunkInfo_t*>(startAddress_);
            freeChunkHeader_->nextChunk     = nullptr;
            freeChunkHeader_->prevChunk     = nullptr;
            freeChunkHeader_->nextFreeChunk = nullptr;
            freeChunkHeader_->prevFreeChunk = nullptr;
            freeChunkHeader_->tag           = Tag;
            freeChunkHeader_->allocStatus   = AllocStatus::UnAllocated;
            freeChunkHeader_->chunkSize     = totalCapacity_;

            externalStartAddress_    = 0;
            externalEndAddress_      = 0;
            externalTotalCapacity_   = 0;
            externalFreeChunkHeader_ = nullptr;
        }

        void resetExternal(void)
        {
            externalFreeChunkHeader_                = reinterpret_cast<SlobChunkInfo_t*>(externalStartAddress_);
            externalFreeChunkHeader_->nextChunk     = nullptr;
            externalFreeChunkHeader_->prevChunk     = nullptr;
            externalFreeChunkHeader_->nextFreeChunk = nullptr;
            externalFreeChunkHeader_->prevFreeChunk = nullptr;
            externalFreeChunkHeader_->tag           = Tag;
            externalFreeChunkHeader_->allocStatus   = AllocStatus::UnAllocated;
            externalFreeChunkHeader_->chunkSize     = externalTotalCapacity_;
        }

        void initExternal(uint64_t startAddress, uint64_t endAddress)
        {
            externalStartAddress_  = startAddress;
            externalEndAddress_    = endAddress;
            externalTotalCapacity_ = (endAddress - startAddress - sizeof(SlobChunkInfo_t));

            resetExternal();
        }

        void* alloc(uint64_t reqSize, bool isExternal = false);

        void* free(void* src);

        uint64_t size(void* src) const
        {
            uint64_t         srcAddress = reinterpret_cast<uint64_t>(src);
            SlobChunkInfo_t* chunkInfo  = reinterpret_cast<SlobChunkInfo_t*>(srcAddress - sizeof(SlobChunkInfo_t));
            return static_cast<uint64_t>(chunkInfo->chunkSize);
        }

        bool isEntryOf(void* src) const
        {
            uint64_t srcAddress = reinterpret_cast<uint64_t>(src);
            if (startAddress_ <= srcAddress && srcAddress <= endAddress_)
            {
                return true;
            }
            else
            {
                if (externalTotalCapacity_ == 0)
                {   
                    return false;
                }
                else
                {
                    return (externalStartAddress_ <= srcAddress && externalStartAddress_ <= externalEndAddress_);
                }
            }
        }

        uint64_t remainSpace(void) const
        {
            uint64_t         remain = 0;
            SlobChunkInfo_t* curr   = freeChunkHeader_;
            while (curr != nullptr)
            {
                remain += curr->chunkSize;
                curr = reinterpret_cast<SlobChunkInfo_t*>(curr->nextFreeChunk);
            }
            return remain;
        }

        uint64_t numFreeChunk(void) const
        {
            uint64_t         remain = 0;
            SlobChunkInfo_t* curr   = freeChunkHeader_;
            while (curr != nullptr)
            {
                remain++;
                curr = reinterpret_cast<SlobChunkInfo_t*>(curr->nextFreeChunk);
            }
            return remain;
        }

        uint64_t totalCapacity() const
        {
            return totalCapacity_;
        }

    private:
        void removeFreeChunk(SlobChunkInfo_t* curr);
        void insertFreeChunk(SlobChunkInfo_t* curr);
        void mergeChunk(SlobChunkInfo_t* dst, SlobChunkInfo_t* src);

    private:
        uint64_t         startAddress_;
        uint64_t         endAddress_;
        uint64_t         totalCapacity_;
        SlobChunkInfo_t* freeChunkHeader_;

        uint64_t         externalStartAddress_;
        uint64_t         externalEndAddress_;
        uint64_t         externalTotalCapacity_;
        SlobChunkInfo_t* externalFreeChunkHeader_;
};

class MUAllocator
{

    private:
        template <typename First, typename... Rest>
        void init(uint64_t startAddress, uint64_t endAddress, First& first, Rest&... rest)
        {
            if constexpr (sizeof...(rest) >= 1)
            {
                uint64_t lastAddress = startAddress + first.totalCapacity();
                MU_ALLOC_ASSERT(lastAddress <= endAddress);
                first.init(startAddress, lastAddress);

                init(lastAddress, endAddress, rest...);
            }
            else
            {
                first.init(startAddress, endAddress);
            }
        }

        template <typename First, typename... Rest>
        void* alloc(uint64_t size, uint64_t alignBit, First& first, Rest&... rest)
        {
            void* result = first.alloc(size);

            if (result)
            {
                return result;
            }
            else
            {
                if constexpr (sizeof...(rest) >= 1)
                {
                    return alloc(size, alignBit, rest...);
                }
            }
            return nullptr;
        }

        template <typename First, typename... Rest>
        void* free(void* src, First& first, Rest&... rest)
        {
            if (first.isEntryOf(src))
            {
                return first.free(src);
            }
            else
            {
                if constexpr (sizeof...(rest) >= 1)
                {
                    return free(src, rest...);
                }
            }
            return nullptr;
        }

        template <typename First, typename... Rest>
        uint64_t poolSize(void* src, First& first, Rest&... rest)
        {
            if (first.isEntryOf(src))
            {
                return first.size(src);
            }
            else
            {
                if constexpr (sizeof...(rest) >= 1)
                {
                    return poolSize(src, rest...);
                }
            }
            return 0;
        }

        template <typename First, typename... Rest>
        void reset(First& first, Rest&... rest)
        {
            first.reset();
            if constexpr (sizeof...(rest) >= 1)
            {
                reset(rest...);
            }
        }

    public:
        explicit MUAllocator(uint64_t header = 0) noexcept
        {
            initMUAllocator(header);
        }

        void initMUAllocator(uint64_t header);

        void* realloc(void* src, uint64_t src_size, uint64_t dst_size, uint64_t alignBit);

        void* calloc(uint64_t count, uint64_t size, uint64_t alignBit);

        void setSharedMem(MuHeader muHeader);

        bool isSharedMemAlloc(void) const
        {
            return allocSharedMem_;
        }

    public:

        #define ALLOCATOR_LIST bitmapAllocator_small, bitmapAllocator_medium, bitmapAllocator_large, slobAllocator // incrementalAllocator//slobAllocator

        void init(uint64_t startAddress, uint64_t endAddress)
        {
            init(startAddress, endAddress, ALLOCATOR_LIST);
        }

        void* alloc(uint64_t size, uint64_t alignBit)
        {
            void*    ret     = alloc(size, alignBit, ALLOCATOR_LIST);
            uint64_t retAddr = reinterpret_cast<uint64_t>(ret);
            return ret;
        }

        void* free(void* src)
        {
            if ((uint64_t)src >= MEM_START(DDR_TASK_CMD))
                return src;

            if ((uint64_t)src < startAddress_)
                return src;

            void* ret = free(src, ALLOCATOR_LIST);
            return ret;
        }

        uint64_t poolSize(void* src)
        {
            return poolSize(src, ALLOCATOR_LIST);
        }

        void reset(void)
        {
            allocSharedMem_ = false;
            return reset(ALLOCATOR_LIST);
        }

        #undef ALLOCATOR_LIST
    private:
        bool     allocSharedMem_;
        uint64_t startAddress_;
        uint64_t endAddress_;
        BitmapAllocator<32, 64>     bitmapAllocator_small;
        BitmapAllocator<KB(4), 64>  bitmapAllocator_medium;
        BitmapAllocator<KB(64), 16> bitmapAllocator_large;
        // IncrementalAllocator incrementalAllocator;
        SlobAllocator slobAllocator;
};

    #endif // NEW ALLOCATOR
#endif     // _SLAVE_

class SharedMemAllocator
{
    public:
        static constexpr uint64_t TOTAL_MEM_SIZE = MEM_SIZE(DDR_SLAVE_MEM_CLST0_SHARED);
        static constexpr uint64_t ALLOC_MEM_SIZE = MB(2);
        static constexpr uint64_t NUM_ALLOC_MEM  = TOTAL_MEM_SIZE / ALLOC_MEM_SIZE;

        static constexpr uint64_t ALLOCATION_FAIL = 64ull;

    public:
        SharedMemAllocator();
        ~SharedMemAllocator();

        uint64_t alloc(uint64_t threadId);
        void     freeThread(uint64_t threadId);
        void     free(uint64_t threadId, uint64_t idx);

    private:
        uint64_t _freeBitmap;
        uint64_t _allocBitmap[MAX_THREAD_PER_CLUSTER];
};

#define UNKNOWN_SRC_SIZE   (0)
#define MU_ALLOC_UNIT_SIZE (64)

#define MuAllocInit(header)                               _muAllocator.initMUAllocator(header);
#define MuAlloc(size)                                     _muAllocator.alloc(size, 0);
#define MuAllocAlign(size, alignBit)                      _muAllocator.alloc(size, alignBit);
#define MuCalloc(cnt, size)                               _muAllocator.calloc(cnt, size, 0);
#define MuCallocAlign(cnt, size, alignBit)                _muAllocator.calloc(cnt, size, alignBit);
#define MuRealloc(src, src_size, dst_size)                _muAllocator.realloc(src, src_size, dst_size, 0);
#define MuReallocAlign(src, src_size, dst_size, alignBit) _muAllocator.realloc(src, src_size, dst_size, alignBit);
#define MuFree(src)                                       _muAllocator.free(src);
#define MuReset()                                                                            \
    do                                                                                       \
    {                                                                                        \
        if (_muAllocator.isSharedMemAlloc())                                                 \
        {                                                                                    \
            MuHeader __muHeader;                                                             \
            __getCsr(MU_CSR_MU_ID, __muHeader.u64);                                          \
            __sync_common(__muHeader.u64, __muHeader.u64, MU_DEVICE_OPCODE_FREE_SHARED_MEM); \
        }                                                                                    \
        _muAllocator.reset();                                                                \
    } while (0);
#define MuRequestSharedMem()                                                              \
    do                                                                                    \
    {                                                                                     \
        MuHeader __muHeader;                                                              \
        __getCsr(MU_CSR_MU_ID, __muHeader.u64);                                           \
        __sync_common(__muHeader.u64, __muHeader.u64, MU_DEVICE_OPCODE_ALLOC_SHARED_MEM); \
        _muAllocator.setSharedMem(__muHeader);                                            \
    } while (0);
