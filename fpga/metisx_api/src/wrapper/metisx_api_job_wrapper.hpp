#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include <functional>
#include <pthread.h>
#include "../util/metisx_logger.hpp"
#include "../metisx_api_const.h"
#include "../metisx_api_map_impl.hpp"

namespace metisx
{

namespace api
{

namespace wrapper
{
typedef struct
{
        std::string dataTag;
        void*       dataAddr;
        uint64_t    dataSize;
} GlobalDataInfo_t;

class Job
{
    public:
        static constexpr uint32_t InvalidJobId          = UINT32_MAX;
    private:
        static constexpr uint32_t InvalidClusterId      = UINT32_MAX;
        static constexpr uint32_t LogSize               = 5000;
        static constexpr uint64_t INVALID_THREAD_BITMAP = 0x0ull;
        static constexpr uint8_t  ASSIGNED_DEVICE       = 1;
        static constexpr uint8_t  UNASSIGNED_DEVICE     = 0;

        bool                              _isInitDone;
        bool                              _monitor;
        bool                              _isReleased;
        uint32_t                          _jobId;
        uint64_t                          _threadBitmap;
        uint32_t                          _numSlaveMu;
        std::vector<GlobalDataInfo_t>     _jobDataList;
        std::vector<std::vector<uint8_t>> _isAssignedDevice;

        metisx::api::util::JobLogger<LogSize> _defaultLogger;

    private:
        void     _printLog();
        uint64_t _allocAdminMuHeader(uint8_t fpgaId, uint8_t mtsCoreId);
        void     _updateThreadBitmapToDevice();

    public:
        void init(ssize_t jobId = InvalidJobId, uint32_t numThread = 0, uint32_t debug = static_cast<uint32_t>(metisx::api::Debug::Disable));
        void release(void);
        void loadProgram(const char* file, uint32_t clusterId = InvalidClusterId, uint8_t fpgaId = 0);

        void* allocJobData(uint64_t size, const char* tag = "");
        void* loadJobData(const char* tag, void* src, uint64_t offset, uint64_t size, void* dst = nullptr);
        void  releaseJobData(void* deviceAddr);

        void activateMonitor(const char* logName = nullptr, std::function<void(metisx::api::util::MxLibPerfLog_t&, metisx::api::util::DevicePerfLog_t&, uint64_t)> cbFunc = nullptr);
        void getJobData(const char* tag, uint64_t* deviceBufAddr, uint64_t* deviceBufSize);

        void*  allocGlobalData(const char* tag, uint64_t size);
        void*  loadGlobalData(const char* tag, void* src, uint64_t offset, uint64_t size, void* dst = nullptr);
        void   releaseGlobalData(const char* tag);
        void   releaseAllGlobalData(void);
        double getSlaveUtil(void);
        void   setThreadBitmap(uint32_t numSlaveMu, uint64_t threadBitmap);

        template <typename... ARGS>
        void map(ARGS... args)
        {
            metisx::api::impl::mxMapImpl(_jobId, args...);
        }

        size_t getJobId(void) const
        {
            return _jobId;
        }

    public:
        Job();
        ~Job();
};
} // namespace wrapper
} // namespace api
} // namespace metisx