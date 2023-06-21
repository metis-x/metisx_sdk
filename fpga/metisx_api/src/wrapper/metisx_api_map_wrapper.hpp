#pragma once
#include "metisx_api_job_wrapper.hpp"
#include "metisx_api_task_wrapper.hpp"
#include "metisx_api_executer_wrapper.hpp"
#include "../util/metisx_exception.hpp"
#include <type_traits>
#include <iostream>

namespace metisx
{

namespace api
{
namespace wrapper
{

enum class ArgType : size_t
{
    Input = 0x1,
    Output = (0x1 << 1),
};



namespace detail
{

const uint64_t InvalidTaskCount = UINT64_MAX;
typedef struct
{
        size_t entrySignature;
        size_t entrySize;
        size_t entryCount;
        size_t entryType;
} mxMallocHeader_t;


enum class TaskRegion : int
{
    Input,
    Output,
};


inline mxMallocHeader_t* _getMxMallocMeta(void* ptr)
{
    uint64_t mxHeaderAddr = reinterpret_cast<uint64_t>(ptr) - sizeof(mxMallocHeader_t);
    return reinterpret_cast<mxMallocHeader_t*>(mxHeaderAddr);
}

inline bool isInputArg(size_t entryType)
{
    return entryType & 0x1;
}

inline bool isOutputArg(size_t entryType)
{
    return (entryType >> 1) & 0x1;
}

template <typename ARG>
bool isPointer(ARG& arg)
{
    return std::is_pointer<ARG>::value;
}

template <typename ARG>
size_t getMinEntryCount(ARG& arg)
{

    size_t entryCount;
    if (isPointer(arg))
    {
        mxMallocHeader_t* mxMallocHeader = _getMxMallocMeta((void*)arg);
        entryCount                       = mxMallocHeader->entryCount;
    }
    else
    {
        entryCount = InvalidTaskCount;
    }
    return entryCount;
}

template <typename ARG, typename... ARGS>
size_t getMinEntryCount(ARG& arg, ARGS&... args)
{
    size_t entryCount;
    if (isPointer(arg))
    {
        mxMallocHeader_t* mxMallocHeader = _getMxMallocMeta((void*)arg);
        entryCount                       = mxMallocHeader->entryCount;
    }
    else
    {
        entryCount = InvalidTaskCount;
    }
    return std::min(entryCount, getMinEntryCount(args...));
}

template <typename ARG>
size_t getInputBufferSize(ARG& arg)
{
    size_t entrySize;
    if (isPointer(arg))
    {
        mxMallocHeader_t* mxMallocHeader = _getMxMallocMeta((void*)arg);
        entrySize                        = mxMallocHeader->entrySize;

        if (isInputArg(mxMallocHeader->entryType))
        {
            if (entrySize > sizeof(size_t))
            {
                entrySize += sizeof(size_t); // point to device ddr
            }
        }
        else
        {
            // output buf
            entrySize = sizeof(void*); // point to device ddr
        }
    }
    else
    {
        entrySize = sizeof(size_t);
    }
    return entrySize;
}

template <typename ARG, typename... ARGS>
size_t getInputBufferSize(ARG& arg, ARGS&... args)
{
    size_t entrySize;
    if (isPointer(arg))
    {
        mxMallocHeader_t* mxMallocHeader = _getMxMallocMeta((void*)arg);
        entrySize                        = mxMallocHeader->entrySize;

        if (isInputArg(mxMallocHeader->entryType))
        {
            if (entrySize > sizeof(size_t))
            {
                entrySize += sizeof(size_t); // point to device ddr
            }
        }
        else
        {
            // output buf
            entrySize = sizeof(void*); // point to device ddr
        }
    }
    else
    {
        entrySize = sizeof(size_t);
    }

    return entrySize + getInputBufferSize(args...);
}

template <typename ARG>
size_t getOutputBufferSize(ARG& arg)
{
    if (isPointer(arg))
    {
        mxMallocHeader_t* mxMallocHeader = _getMxMallocMeta((void*)arg);
        if (isInputArg(mxMallocHeader->entryType))
        {
            return 0;
        }
        else
        {
            // output buf
            return mxMallocHeader->entrySize;
        }
    }
    return 0;
}

template <typename ARG, typename... ARGS>
size_t getOutputBufferSize(ARG& arg, ARGS&... args)
{
    size_t entrySize = 0;
    if (isPointer(arg))
    {
        mxMallocHeader_t* mxMallocHeader = _getMxMallocMeta((void*)arg);
        if (isInputArg(mxMallocHeader->entryType))
        {
            return getOutputBufferSize(args...);
        }
        else
        {
            // output buf
            return mxMallocHeader->entrySize + getOutputBufferSize(args...);
        }
    }
    return getOutputBufferSize(args...);
}

template <typename ARG>
void setupArg(metisx::api::wrapper::Task* taskPtr, void* hostTmpBuf, uint64_t inputBufferOffset, uint64_t inputBufferAddr, uint64_t outputBufferAddr, size_t idx, ARG& arg)
{
    if (isPointer(arg))
    {
        mxMallocHeader_t* mxMallocHeader = _getMxMallocMeta((void*)arg);

        uint64_t hostArgAddr = ((uint64_t)arg + idx * mxMallocHeader->entrySize);

        if (isOutputArg(mxMallocHeader->entryType))
        {
            if (isInputArg(mxMallocHeader->entryType))
            {
                // in/out arg
                memcpy((void*)hostTmpBuf, (void*)&inputBufferAddr, mxMallocHeader->entrySize);
                memcpy((void*)((uint64_t)hostTmpBuf + inputBufferOffset), (void*)hostArgAddr, mxMallocHeader->entrySize);

                taskPtr->pushToInOutBufQueue(inputBufferAddr);
            }
            else
            {
                // output only ARG
                memcpy((void*)hostTmpBuf, (void*)&outputBufferAddr, sizeof(uint64_t));
                outputBufferAddr += mxMallocHeader->entrySize;
            }
        }
        else
        {
            // is Input only Arg
            memcpy((void*)hostTmpBuf, (void*)hostArgAddr, mxMallocHeader->entrySize);
        }
    }
    else
    {
        memcpy(hostTmpBuf, (void*)&arg, sizeof(arg));
    }
}

template <typename ARG, typename... ARGS>
void setupArg(metisx::api::wrapper::Task* taskPtr, void* hostTmpBuf, uint64_t inputBufferOffset, uint64_t inputBufferAddr, uint64_t outputBufferAddr, size_t idx, ARG& arg, ARGS&... args)
{
    if (isPointer(arg))
    {
        mxMallocHeader_t* mxMallocHeader = _getMxMallocMeta((void*)arg);

        uint64_t hostArgAddr = ((uint64_t)arg + idx * mxMallocHeader->entrySize);

        if (isOutputArg(mxMallocHeader->entryType))
        {
            if (isInputArg(mxMallocHeader->entryType))
            {
                // in/out arg
                memcpy((void*)hostTmpBuf, (void*)&inputBufferAddr, mxMallocHeader->entrySize);
                memcpy((void*)((uint64_t)hostTmpBuf + inputBufferOffset), (void*)hostArgAddr, mxMallocHeader->entrySize);

                taskPtr->pushToInOutBufQueue(inputBufferAddr);

                inputBufferAddr += mxMallocHeader->entrySize;
                inputBufferOffset += mxMallocHeader->entrySize;
            }
            else
            {
                // output only ARG
                memcpy((void*)hostTmpBuf, (void*)&outputBufferAddr, sizeof(uint64_t));
                outputBufferAddr += mxMallocHeader->entrySize;
            }
        }
        else
        {
            // is Input only Arg
            memcpy((void*)hostTmpBuf, (void*)hostArgAddr, mxMallocHeader->entrySize);
        }
    }
    else
    {
        memcpy(hostTmpBuf, (void*)&arg, sizeof(arg));
    }
    hostTmpBuf = (void*)((uint64_t)hostTmpBuf + sizeof(size_t));
    setupArg(taskPtr, hostTmpBuf, inputBufferOffset, inputBufferAddr, outputBufferAddr, idx, args...);
}

template <typename ARG>
void getOutput(metisx::api::wrapper::Task* taskPtr, void* outputBuf, size_t idx, ARG& arg)
{
    if (isPointer(arg))
    {
        mxMallocHeader_t* mxMallocHeader = _getMxMallocMeta((void*)arg);
        uint64_t          hostArgAddr    = ((uint64_t)arg + idx * mxMallocHeader->entrySize);
        if (isInputArg(mxMallocHeader->entryType) && isOutputArg(mxMallocHeader->entryType))
        {
            uint64_t deviceInputBufAddr = taskPtr->popFromInOutBufQueue();
            memcpy((void*)hostArgAddr, (void*)deviceInputBufAddr, mxMallocHeader->entrySize);
        }
        else if (isOutputArg(mxMallocHeader->entryType))
        {
            memcpy((void*)hostArgAddr, outputBuf, mxMallocHeader->entrySize);
        }
    }
}

template <typename ARG, typename... ARGS>
void getOutput(metisx::api::wrapper::Task* taskPtr, void* outputBuf, size_t idx, ARG& arg, ARGS&... args)
{
    if (isPointer(arg))
    {
        mxMallocHeader_t* mxMallocHeader = _getMxMallocMeta((void*)arg);
        uint64_t          hostArgAddr    = ((uint64_t)arg + idx * mxMallocHeader->entrySize);
        if (isInputArg(mxMallocHeader->entryType) && isOutputArg(mxMallocHeader->entryType))
        {
            uint64_t deviceInputBufAddr = taskPtr->popFromInOutBufQueue();
            taskPtr->readDeviceMemory((void*)hostArgAddr, (void*)deviceInputBufAddr, 0, mxMallocHeader->entrySize);
        }
        else if (isOutputArg(mxMallocHeader->entryType))
        {
            memcpy((void*)hostArgAddr, outputBuf, mxMallocHeader->entrySize);
            outputBuf = (void*)((uint64_t)outputBuf + mxMallocHeader->entrySize);
        }
    }
    getOutput(taskPtr, outputBuf, idx, args...);
}

} // namespace detail

void* mxMallocDecl(uint64_t entrySize, uint64_t entryCount, uint64_t type);
void  mxFreeDecl(void* address);

template <typename String, typename... ARGS>
void mxMap(const String& fileName, ARGS... args)
{
    // init Job
    auto offloadJob = new metisx::api::wrapper::Job();
    // init TaskExecutor
    auto async_executor = new metisx::api::wrapper::MetisExecuter();

    size_t nargs     = sizeof...(args);
    size_t taskCount = detail::getMinEntryCount(args...);
    size_t jobId;
    try
    {
        offloadJob->init();
        offloadJob->loadProgram(fileName);
        jobId = offloadJob->getJobId();
        async_executor->init(taskCount);
    }
    catch (const metisx::api::util::MetisException& err)
    {
        std::cerr << err.what();
    }

    if (taskCount < 0)
    {
        taskCount = 1;
    }

    size_t argSize           = nargs * sizeof(size_t);
    size_t taskInputBufSize  = detail::getInputBufferSize(args...);
    size_t taskOutputBufSize = detail::getOutputBufferSize(args...);
    size_t taskIssueCount    = 0;
    size_t taskDoneCount     = 0;

    while (taskDoneCount != taskCount)
    {
        while (taskIssueCount < taskCount)
        {
            auto task = new metisx::api::wrapper::Task();
            bool ret  = task->init(jobId, 0);
            if (!ret)
            {
                delete task;
                break;
            }

            if (nargs > 0)
            {
                void* taskInputBufPtr;
                void* taskOutputBufPtr;
                void* tempInputBuf;

                taskInputBufPtr = task->allocTaskBuffer(taskInputBufSize, static_cast<int>(detail::TaskRegion::Input));
                if (!taskInputBufPtr)
                {
                    delete task;
                    break;
                }

                tempInputBuf = malloc(taskInputBufSize);
                if (!tempInputBuf)
                {
                    delete task;
                    break;
                }

                if (taskOutputBufSize > 0)
                {
                    taskOutputBufPtr = task->allocTaskBuffer(taskOutputBufSize, static_cast<int>(detail::TaskRegion::Output));
                    if (!taskOutputBufPtr)
                    {
                        delete task;
                        break;
                    }
                }

                detail::setupArg(task, tempInputBuf, argSize, (uint64_t)taskInputBufPtr + argSize, (uint64_t)taskOutputBufPtr, taskIssueCount, args...);
                task->loadTaskCmd(tempInputBuf, 0, taskInputBufSize, taskInputBufPtr, static_cast<int>(detail::TaskRegion::Input), nargs);
                free(tempInputBuf);
            }

            task->registerTaskTag((uint64_t)taskIssueCount);
            async_executor->enqueueTask(task, 0);
            taskIssueCount++;
        }

        while (taskDoneCount < taskCount)
        {
            auto doneTask = async_executor->dequeueTask();
            if (doneTask != NULL)
            {

                uint64_t taskIdx = doneTask->getTaskTag();

                uint64_t outputBufSize = doneTask->getOutputBufSize();
                if (outputBufSize > 0)
                {
                    void* outputBuf = malloc(outputBufSize);
                    doneTask->getResult(outputBuf);
                    detail::getOutput(doneTask, outputBuf, taskIdx, args...);
                    free(outputBuf);
                }
                else
                {
                    if (doneTask->getInOutBufQueueSize() > 0)
                    {
                        detail::getOutput(doneTask, nullptr, taskIdx, args...);
                    }
                }

                delete doneTask;
                taskDoneCount++;
            }
            else
            {
                break;
            }
        }
    }

    delete offloadJob;
    delete async_executor;
}

template <typename String>
void mxMap(const String& fileName)
{
    mxMap(fileName, nullptr);
}

} // namespace wrapper
} // namespace api
} // namespace metisx