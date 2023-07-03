#pragma once
#include <stdint.h>
#include "metisx_api_map_impl_detail.hpp"
#include "./wrapper/metisx_api_executer_wrapper.hpp"
namespace metisx
{
namespace api
{
namespace impl
{

enum class DeviceDDR : uint32_t
{
    taskRegion,
    HeapRegion,
};

void mxMapYield();

template <typename Job, typename... ARGS>
void mxMapImpl(const Job jobId, ARGS... args)
{
    std::size_t argc = sizeof...(args);
    if (argc == 0)
    {
        return;
    }

    std::size_t       argParamSize    = argc * sizeof(uint64_t);
    std::size_t       argSize         = detail::getArgSize(args...);
    std::size_t       inputBufSize    = detail::getInputArgSize(args...);
    std::size_t       outputBufOffset = inputBufSize;
    std::size_t       outputBufSize   = detail::getOutputArgSize(args...);
    std::size_t       taskCount       = detail::getMinEntryCount(args...);
    uint64_t          taskDoneCount   = 0;
    uint64_t          taskIssueCount  = 0;
    detail::ArgInfo_t argInfo;

    if (taskCount == detail::InvalidTaskCount) // only constant args
    {
        taskCount = 1;
    }

    auto async_executor = std::make_unique<metisx::api::wrapper::MetisExecuter>();
    async_executor->init(taskCount);

    while (taskDoneCount != taskCount)
    {
        bool checkSleep = true;
        while (taskIssueCount < taskCount)
        {
            auto        task = new metisx::api::wrapper::Task();
            std::size_t ret  = task->init(jobId, false);
            if (ret == static_cast<std::size_t>(metisx::api::wrapper::TaskInitStatus::Fail))
            {
                delete task;
                break;
            }
            void* taskBufPtr = task->allocTaskBuffer(argSize);
            if (!taskBufPtr)
            {
                delete task;
                break;
            }
            void* hostArgBuf = malloc(argSize);
            if (!hostArgBuf)
            {
                delete task;
                break;
            }

            argInfo.inputBufOffset  = argParamSize;
            argInfo.outputBufOffset = outputBufOffset;
            argInfo.taskIdx         = taskIssueCount;
            argInfo.hostBuf         = hostArgBuf;
            argInfo.taskBufAddress  = reinterpret_cast<uint64_t>(taskBufPtr);
            argInfo.argc            = 0;

            detail::setupArg(&argInfo, args...);
            task->setTaskInputArg(hostArgBuf, inputBufSize, taskBufPtr, argc);
            uint64_t outputBufDeviceAddr = reinterpret_cast<uint64_t>(taskBufPtr) + outputBufOffset;
            task->setTaskOutputArg(reinterpret_cast<void*>(outputBufDeviceAddr), outputBufSize);
            task->registerTaskTag(taskIssueCount);
            async_executor->enqueueTask(task);
            free(hostArgBuf);
            checkSleep = false;
            taskIssueCount++;
        }

        while (taskDoneCount < taskCount)
        {
            auto doneTask = async_executor->dequeueTask();
            if (doneTask != NULL)
            {
                uint64_t taskIdx        = doneTask->getTaskTag();
                uint64_t inputBufAddr   = doneTask->inputBufAddr();
                uint64_t outputBufAddr  = doneTask->outputBufAddr();
                uint64_t outputBufSize  = doneTask->outputBufSize();
                uint64_t inputBufOffset = argParamSize;

                void* outputTempBuf = nullptr;
                if (outputBufSize != 0)
                {
                    outputTempBuf = malloc(outputBufSize);
                    doneTask->taskResult(outputTempBuf);
                }

                detail::getOutput(doneTask, taskIdx, inputBufAddr, inputBufOffset, outputTempBuf, taskIdx, args...);

                if (outputTempBuf)
                {
                    free(outputTempBuf);
                }
                delete doneTask;
                checkSleep = false;
                taskDoneCount++;
            }
            else
            {
                break;
            }
        }
        if (checkSleep)
        {
            mxMapYield();
        }
    }
    async_executor.reset();
}
} // namespace Impl
} // namespace api
} // namespace metisx