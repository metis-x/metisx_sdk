#pragma once
#include <stdint.h>
#include <map>
#include <memory>

#include "metisx_api_task_wrapper.hpp"
#include "../util/metisx_queue.hpp"

namespace metisx
{

namespace api
{

namespace wrapper
{

class MetisExecuter
{
    private:
        const uint64_t        MU_HEADER_HOST_MASK = 0xFFFFFFFF00000000ull;
        static const uint64_t DefaultBufSize      = 128 * 1024;

    private:
        std::map<uint64_t, metisx::api::wrapper::Task*>                _pendingTaskMap;
        std::unique_ptr<metisx::api::util::MxQueue<uint64_t>> _executionDoneQueue;
        uint32_t                                              _maxOutstandingCnt;
        uint32_t                                              _curOutstandingCnt;

    public:
        void               init(uint32_t maxNumOutstandingTask);
        bool               enqueueTask(metisx::api::wrapper::Task* mxTaskPtr, uint64_t outputBufSize = DefaultBufSize);
        metisx::api::wrapper::Task* dequeueTask(void);

    public:
        MetisExecuter();
        ~MetisExecuter();
};

} // namespace wrapper
} // namespace api

} // namespace metisx