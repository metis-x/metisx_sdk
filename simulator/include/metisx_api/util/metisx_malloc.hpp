#pragma once
#include <vector>
#include <stdint.h>
#include <memory>
#include <mutex>

#define SUPPORT_GRANULARITY (0)
#define DEBUG_MX_MALLOC     (0)

#if (!SUPPORT_GRANULARITY)
    #include <unordered_map>
#endif

namespace metisx
{

namespace api
{

namespace util
{

class MxMalloc
{

    private:
        typedef struct
        {
                uint32_t lastFreeChunkIdx;
                uint32_t firstFreeChunkIdx;
                uint64_t numEntry;
        } FreeListHeadGranularity_t;

        typedef struct
        {
                union
                {
                        uint64_t qw1;

                        struct
                        {
                                uint64_t prevIdx        : 24;
                                uint64_t nextIdx        : 24;
                                uint64_t chunkSizeLower : 16;
                        };
                };

                union
                {
                        uint64_t qw2;

                        struct
                        {
                                uint64_t prevFreeChunkIdx : 24;
                                uint64_t nextFreeChunkIdx : 24;
                                uint64_t chunkSizeUpper   : 8;
                                uint64_t allocated        : 1;
                                uint64_t rsvd             : 7;
                        };
                };
        } MxMallocMetaGranularity_t;

        typedef struct
        {
                void*  prevMetaChunk;
                void*  nextMetaChunk;
                void*  prevFreeListChunk;
                void*  nextFreeListChunk;
                size_t chunkIdx;
                size_t chunkSize;
        } MxMallocMeta_t;

        typedef struct
        {
                MxMallocMeta_t* firstFreeChunk;
                MxMallocMeta_t* lastFreeChunk;
                size_t          numEntry;
        } FreeListHead_t;

        typedef struct
        {
                size_t lowerBound;
                size_t upperBound;
        } BinRange_t;

    private:
        MxMallocMetaGranularity_t*                 _chunkListGranularity;
        MxMallocMeta_t*                            _metaChunkHead;
        std::unordered_map<void*, MxMallocMeta_t*> _allocPtrMap;

        std::vector<FreeListHead_t>            _binHeaderList;
        std::vector<FreeListHeadGranularity_t> _binHeaderListGranularity;
        std::vector<BinRange_t>                _binRangeList;
        size_t                                 _capacity;
        size_t                                 _granularity;
        size_t                                 _numTotalChunk;
        size_t                                 _numBin;
        size_t                                 _baseAddr;
        size_t                                 _allocCount;
        size_t                                 _remainCapacity;
        bool                                   _debug;
        std::unique_ptr<std::mutex>            _mallocLock;

        const uint32_t MAX_NUM_CHUNK      = 0xFFFFFF;
        const uint64_t INVALID_CHUNK_ADDR = 0x0ull;
        const uint64_t END_CHUNK_ADDR     = 0xFFFFFFFFFFFFFFFFull;
        const uint64_t MAX_CHUNK_SIZE     = 0xFFFFFFFFFFFFFFFFull;

        const bool UNALLOCATED = 0;
        const bool ALLOCATED   = 1;

    public:
        MxMalloc(size_t baseAddr, size_t capacity, size_t granularity = 0, bool debug = false);
        ~MxMalloc();

    public:
        void* metisx_malloc(size_t reqSize);
        void* mx_calloc(size_t numEntry, size_t dataTypeSize);
        void  mx_free(void* ptr);

    private:
        uint32_t _findBinIndex(size_t size);

        uint32_t _searchBestFitIndexGranularity(size_t reqSize);
        void     _mergeFreeChunkGranularity(uint32_t freeChunkIdx, uint32_t mergeChunkIdx);
        void     _addToFreeListGranularity(uint32_t binI, uint32_t chunkIndex);
        void     _removeFromFreeListGranularity(uint32_t binIndex, uint32_t chunkIndex);

        MxMallocMeta_t* _searchBestFitIndex(size_t reqSize);
        void            _mergeFreeChunk(MxMallocMeta_t* freeChunk, MxMallocMeta_t* mergeChunk);
        void            _addToFreeList(uint32_t binIndex, MxMallocMeta_t* chunk);
        void            _removeFromFreeList(uint32_t binIndex, MxMallocMeta_t* chunk);

        void* _mallocGranularity(size_t reqSize);
        void* _mallocNormal(size_t reqSize);

        void _freeGranularity(void* ptr);
        void _freeNormal(void* ptr);

        inline uint32_t allocCount(void)
        {
            return _allocCount;
        }

        inline size_t capacity(void)
        {
            return _capacity;
        }

        inline size_t baseAddr(void)
        {
            return _baseAddr;
        }

        inline size_t granularity(void)
        {
            return _granularity;
        }

        inline size_t endAddr(void)
        {
            return _baseAddr + _capacity;
        }
};

} // namespace util
} // namespace api
} // namespace metisx