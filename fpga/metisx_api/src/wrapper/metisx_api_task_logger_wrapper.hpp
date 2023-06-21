#pragma once
#include "../util/metisx_logger.hpp"
#include <functional>
#include <string>

namespace metisx
{

namespace api
{
namespace wrapper
{

class TaskLogger
{
    private:
        uint32_t                                                                                               _jobId;
        std::function<void(metisx::api::util::MxLibPerfLog_t&, metisx::api::util::DevicePerfLog_t&, uint64_t)> _cbFunc;
        metisx::api::util::MxLibPerfLog_t                                                                      _timePointInfoList;
        metisx::api::util::DevicePerfLog_t                                                                     _devicePerfInfoList;
        uint64_t                                                                                               _slaveUtil;

    private:
        uint32_t calculateElapsedCycle(uint32_t start, uint32_t end);

    public:
#define SET_TIMER() setTimer(__FUNCTION__, __LINE__)

        void setTimer(std::string funcName, int lineNum);

        bool isLoggerEnabled()
        {
            if (_cbFunc == nullptr)
            {
                return false;
            }
            return true;
        }

        void setMuLatency(void* debugInfo, uint64_t muHeader);
        void doneTimer(void);

    public:
        TaskLogger(uint32_t jobId);

        ~TaskLogger()
        {
        }
};
} // namespace wrapper

} // namespace api
} // namespace metisx
