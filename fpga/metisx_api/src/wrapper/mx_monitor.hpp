#pragma once

#include <unordered_map>
#include <chrono>
#include <memory>
#include <mutex>
#include <functional>
#include "../util/metisx_logger.hpp"
#include "../manager/metisx_api_manager.hpp"

namespace metisx
{

namespace api
{

class MxMonitor
{

    private:
        typedef struct
        {
                std::chrono::steady_clock::time_point taskInitPoint;
                std::chrono::steady_clock::time_point taskStartPoint;
                std::chrono::steady_clock::time_point taskDonePoint;
        } MxMonitorTaskLog_t;

        typedef struct
        {
                bool                                  isAlloc;
                std::chrono::steady_clock::time_point jobInitTimePoint;
                std::chrono::steady_clock::time_point jobReleaseTimePoint;
        } MxJobLog_t;

        const uint32_t LOG_LIMIT            = 1000;
        const uint8_t  ALLOCATED            = 1;
        const uint8_t  UNALLOCATED          = 0;
        const double   TASK_TIME_OUT_MAX    = 500;
        const double   INVALID_ELAPSED_TIME = 0;
        const ssize_t  MX_MONITOR_FAIL      = -1;
        const ssize_t  MX_MONITOR_SUCCESS   = 1;

    private:
        bool                                                                                                                         _isInitDone;
        std::unordered_map<uint32_t, std::function<void(metisx::api::util::MxLibPerfLog_t&, metisx::api::util::DevicePerfLog_t&, uint64_t)>> _jobCallbackFuncList;
        std::unique_ptr<std::mutex>                                                                                                  _mxMonitorMutex;

    private:
        MxMonitor();
        ~MxMonitor();

    private:
        friend MxMonitor& metisx::api::manager::GetInstance<MxMonitor>();

    public:
        uint32_t                                                                                       getTotalOutstandingCmdCount();
        uint32_t                                                                                       getOutstandingCmdCount(uint32_t jobId);
        ssize_t                                                                                        enableJobMonitor(uint32_t jobId, std::function<void(metisx::api::util::MxLibPerfLog_t&, metisx::api::util::DevicePerfLog_t&, uint64_t)> cbFunc);
        void                                                                                           disableJobMonitor(uint32_t jobId);
        std::function<void(metisx::api::util::MxLibPerfLog_t&, metisx::api::util::DevicePerfLog_t&, uint64_t)> getCallBackFunc(uint32_t jobId);
};

} // namespace api

} // namespace metisx
