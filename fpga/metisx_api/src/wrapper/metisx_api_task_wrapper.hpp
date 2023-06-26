#pragma once

#include <vector>
#include <chrono>
#include <queue>
#include <string>
#include <functional>

namespace metisx
{
namespace api
{
namespace wrapper
{
enum class TaskInitStatus : ssize_t
{
    Fail    = -1,
    Success = 1,
};

class Task
{
    public:
        enum Result : ssize_t
        {
            Success        = 1,
            BufferOverFlow = 2,
        };

    private:
        typedef struct
        {
                uint64_t inputParamAddr;
                uint64_t inputParamSize;
                uint64_t outputBufAddr;
                uint64_t outputBufSize;
        } TaskBufInfo_t;

        typedef union
        {
                uint64_t qw[8];

                struct
                {
                        TaskBufInfo_t taskBufInfo;
                        uint64_t      adminCmdOpcode : 4;
                        uint64_t      numSlaveMu     : 4;
                        uint64_t      rsvd           : 58;
                        uint64_t      threadBitmap; // qw4
                        uint64_t      argc;
                        uint64_t      rsvd2;
                };
        } MxTaskCmd_t;

        typedef union
        {
                uint64_t qw[8];

                struct
                {
                        uint32_t outputOverFlow;
                        uint32_t adminOutstandingCnt;
                        uint32_t adminStartCycle;
                        uint32_t adminEndCycle;
                        uint32_t masterStartCycle;
                        uint32_t masterEndCycle;
                        uint32_t slaveStartCycle;
                        uint32_t slaveEndCycle;
                        uint32_t slaveExecStartCycle;
                        uint32_t slaveExecEndCycle;
                        uint64_t rsvd0;
                        uint64_t rsvd1;
                        uint64_t rsvd2;
                };
        } MxTaskDbgInfo_t;

    private:
        uint64_t             _muHeader;
        MxTaskCmd_t          _taskCmd;
        MxTaskDbgInfo_t      _debugInfo;
        uint64_t             _paramBufAddr;
        bool                 _isReleased;
        bool                 _isDebug;
        int                  _isSync;
        uint64_t             _taskTag;
        std::vector<void*>   _taskInputBufList;
        std::queue<uint64_t> _taskInOutBufQueue;
        void*                taskLogger;

        const uint64_t        InvalidTaskTag       = UINT64_MAX;
        static const uint64_t DefaultOutputBufSize = 128 * 1024;

    private:
        void  _allocateOutputBuf(uint64_t outputBufSize);
        void  _syncExecute(void);
        void  _asyncExecute(void* doneQueuePtr);
        void* _allocDeviceBuffer(uint32_t regionId, uint64_t size);

    public:
        ssize_t init(uint32_t jobId, bool isSync = true, uint64_t muHeader = 0);
        bool    execute(int isSync = 1, uint64_t outputBufSize = DefaultOutputBufSize, void* doneQueuePtr = nullptr);
        void    release();

        void*   allocTaskBuffer(uint64_t size, uint32_t regionId = 0);
        void*   loadTaskData(void* src, uint64_t offset, uint64_t size, void* dst = nullptr, uint32_t regionId = 0);
        void    getJobDataPtr(const char* tag, uint64_t* addr, uint64_t* dataSize);
        void    getGlobalDataPtr(const char* tag, uint64_t* addr, uint64_t* dataSize);
        void    releaseAllTaskData();
        void    loadTaskCmd(void* src, uint64_t offset, uint64_t size, void* deviceDst = nullptr, uint32_t regionId = 0, uint64_t argc = 0);
        bool    allocateTaskCmd(uint64_t size);
        ssize_t getResult(void* dst, uint64_t size = 0);

        void     readDeviceMemory(void* dst, void* src, uint64_t offset, uint64_t size);
        uint64_t getOutputBufAddr();
        uint64_t getOutputBufSize();
        uint64_t getMuHeader();
        void     registerTaskTag(uint64_t tag);
        uint64_t getTaskTag(void);
        void     pushToInOutBufQueue(uint64_t deviceAddrPtr);
        uint64_t popFromInOutBufQueue();
        uint64_t getInOutBufQueueSize();

    public:
        Task();
        ~Task();
};

} // namespace wrapper
} // namespace api

} // namespace metisx