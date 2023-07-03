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
            BufferOverFlow = -1,
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
        uint64_t           muHeader_;
        MxTaskCmd_t        taskCmd_;
        MxTaskDbgInfo_t    _debugInfo;
        bool               _isReleased;
        bool               _isDebug;
        int                _isSync;
        uint64_t           _taskTag;
        std::vector<void*> taskBufList_;
        void*              taskLogger;

        const uint64_t InvalidTaskTag = UINT64_MAX;

    private:
        void  _syncExecute(void);
        void  _asyncExecute(void* doneQueuePtr);
        void* _allocDeviceBuffer(uint64_t size);

    public:
        ssize_t init(uint32_t jobId, bool isSync = true, uint64_t muHeader = 0);
        bool    execute(int isSync = 1, uint64_t outputBufSize = 0, void* doneQueuePtr = nullptr);
        void    release();
        void*   allocTaskBuffer(uint64_t size);
        void*   loadTaskBuffer(void* src, uint64_t offset, uint64_t size, void* dst = nullptr);
        void    getJobDataPtr(const char* tag, uint64_t* addr, uint64_t* dataSize);
        void    getGlobalDataPtr(const char* tag, uint64_t* addr, uint64_t* dataSize);
        void    releaseAllTaskData();
        void    setTaskInputArg(void* inputArg, uint64_t inputArgSize, void* deviceDst = nullptr, uint64_t argc = 0);
        void    setTaskOutputArg(void* deviceAddr, uint64_t outputArgSize);
        ssize_t taskResult(void* dst, uint64_t size = 0);
        void    readTaskBuffer(void* dst, void* src, uint64_t offset, uint64_t size);

        void registerTaskTag(uint64_t tag)
        {
            _taskTag = tag;
        }

        uint64_t getMuHeader()
        {
            return muHeader_;
        }

        uint64_t getTaskTag(void)
        {
            return _taskTag;
        }
        
        uint64_t inputBufAddr()
        {
            return taskCmd_.taskBufInfo.inputParamAddr;
        }

        uint64_t inputBufSize()
        {
            return taskCmd_.taskBufInfo.inputParamSize;
        }

        uint64_t outputBufAddr()
        {
            return taskCmd_.taskBufInfo.outputBufAddr;
        }

        uint64_t outputBufSize()
        {
            return taskCmd_.taskBufInfo.outputBufSize;
        }

    public:
        Task();
        ~Task();
};

} // namespace wrapper
} // namespace api

} // namespace metisx