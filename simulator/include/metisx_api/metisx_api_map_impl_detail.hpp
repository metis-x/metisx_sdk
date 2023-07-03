#pragma once
#include <stdint.h>
#include <cstring>
#include "metisx_api_metisx_ptr.hpp"
#include "./wrapper/metisx_api_task_wrapper.hpp"
namespace metisx
{
namespace api
{
namespace impl
{
namespace detail
{

const uint64_t InvalidTaskCount = UINT64_MAX;

template <typename T>
class IsMetisxPtrHelper
{
    public:
        static const bool value = false;
};

template <typename T>
class IsMetisxPtrHelper<metisx::api::Metisx_ptr<T>>
{
    public:
        static const bool value = true;
};

template <typename T>
class IsMetisxPtr : public IsMetisxPtrHelper<T>
{
};

template <typename T>
ArgType getArgType(metisx::api::Metisx_ptr<T> metisxPtr)
{
    metisx::api::detail::MetisxPtrHeader_t* header = reinterpret_cast<metisx::api::detail::MetisxPtrHeader_t*>(reinterpret_cast<uint64_t>(metisxPtr.get()) - sizeof(metisx::api::detail::MetisxPtrHeader_t));
    return header->type;
}

template <typename T>
ArgType getArgType(T metisxPtr)
{
    return ArgType::None;
}

template <typename T>
size_t getEntrySize(metisx::api::Metisx_ptr<T> metisxPtr)
{
    metisx::api::detail::MetisxPtrHeader_t* header = reinterpret_cast<metisx::api::detail::MetisxPtrHeader_t*>(reinterpret_cast<uint64_t>(metisxPtr.get()) - sizeof(metisx::api::detail::MetisxPtrHeader_t));
    return header->entrySize;
}

template <typename T>
size_t getEntrySize(T metisxPtr)
{
    return 0;
}

template <typename T>
size_t getEntryCount(metisx::api::Metisx_ptr<T> metisxPtr)
{
    metisx::api::detail::MetisxPtrHeader_t* header = reinterpret_cast<metisx::api::detail::MetisxPtrHeader_t*>(reinterpret_cast<uint64_t>(metisxPtr.get()) - sizeof(metisx::api::detail::MetisxPtrHeader_t));
    return header->entryCount;
}

template <typename T>
size_t getEntryCount(T metisxPtr)
{
    return 0;
}

template <typename T>
bool isInputArg(metisx::api::Metisx_ptr<T> metisxPtr)
{
    ArgType type = getArgType(metisxPtr);
    return (type == ArgType::Input || type == ArgType::Both);
}

template <typename T>
bool isInputArg(T metisxPtr)
{
    return false;
}

template <typename T>
bool isOutputArg(metisx::api::Metisx_ptr<T> metisxPtr)
{
    ArgType type = getArgType(metisxPtr);
    return (type == ArgType::Output || type == ArgType::Both);
}

template <typename T>
bool isOutputArg(T metisxPtr)
{
    return false;
}

template <typename T>
void* getArgPtr(metisx::api::Metisx_ptr<T> metisxPtr)
{
    return reinterpret_cast<void*>(metisxPtr.get());
}

template <typename T>
void* getArgPtr(T metisxPtr)
{
    return nullptr;
}

template <typename ARG>
std::size_t getMinEntryCount(ARG& arg)
{
    if (IsMetisxPtr<ARG>::value)
    {
        return getEntryCount(arg);
    }
    return InvalidTaskCount;
}

template <typename ARG, typename... ARGS>
std::size_t getMinEntryCount(ARG& arg, ARGS&... args)
{
    std::size_t entryCount = InvalidTaskCount;
    if (IsMetisxPtr<ARG>::value)
    {
        entryCount = getEntryCount(arg);
    }
    return std::min(entryCount, getMinEntryCount(args...));
}

template <typename ARG>
std::size_t getArgSize(ARG& arg)
{
    std::size_t entrySize;
    if (IsMetisxPtr<ARG>::value)
    {
        entrySize = getEntrySize(arg) + sizeof(ARG*);
    }
    else
    {
        entrySize = sizeof(uint64_t);
    }
    return entrySize;
}

template <typename ARG, typename... ARGS>
std::size_t getArgSize(ARG& arg, ARGS&... args)
{
    std::size_t entrySize;
    if (IsMetisxPtr<ARG>::value)
    {
        entrySize = getEntrySize(arg) + sizeof(ARG*);
    }
    else
    {
        entrySize = sizeof(uint64_t);
    }
    return entrySize + getArgSize(args...);
}

inline void* increasePtrBySize(void* ptr, size_t size)
{
    uint64_t ptrAddress = reinterpret_cast<uint64_t>(ptr);
    ptrAddress += size;
    return reinterpret_cast<void*>(ptrAddress);
}

template <typename ARG>
std::size_t getInputArgSize(ARG& arg)
{
    std::size_t entrySize = 0;
    if (IsMetisxPtr<ARG>::value)
    {
        if (isInputArg(arg))
        {
            entrySize = getEntrySize(arg) + sizeof(ARG*);
        }
        else
        {
            entrySize = sizeof(uint64_t);
        }
    }
    else
    {
        entrySize = sizeof(uint64_t);
    }
    return entrySize;
}

template <typename ARG, typename... ARGS>
std::size_t getInputArgSize(ARG& arg, ARGS&... args)
{
    std::size_t entrySize = 0;
    if (IsMetisxPtr<ARG>::value)
    {
        if (isInputArg(arg))
        {
            entrySize = getEntrySize(arg) + sizeof(ARG*);
        }
        else
        {
            entrySize = sizeof(uint64_t);
        }
    }
    else
    {
        entrySize = sizeof(uint64_t);
    }
    return entrySize + getInputArgSize(args...);
}

template <typename ARG>
std::size_t getOutputArgSize(ARG& arg)
{
    std::size_t entrySize = 0;
    if (IsMetisxPtr<ARG>::value)
    {
        if (isOutputArg(arg) && !isInputArg(arg))
        {
            entrySize = getEntrySize(arg);
        }
    }
    return entrySize;
}

template <typename ARG, typename... ARGS>
std::size_t getOutputArgSize(ARG& arg, ARGS&... args)
{
    std::size_t entrySize = 0;
    if (IsMetisxPtr<ARG>::value)
    {
        if (isOutputArg(arg) && !isInputArg(arg))
        {
            entrySize = getEntrySize(arg);
        }
    }
    return entrySize + getOutputArgSize(args...);
}

typedef struct
{
        int      argc;
        int      taskIdx;
        void*    hostBuf;
        uint64_t taskBufAddress;
        uint64_t inputBufOffset;
        uint64_t outputBufOffset;
} ArgInfo_t;

template <typename ARG>
void setupArg(ArgInfo_t* argInfo, ARG& arg)
{
    void* hostArgv = increasePtrBySize(argInfo->hostBuf, argInfo->argc * sizeof(uint64_t));
    if (IsMetisxPtr<ARG>::value)
    {
        if (isInputArg(arg))
        {
            void*    hostArgvPtr     = increasePtrBySize(argInfo->hostBuf, argInfo->inputBufOffset);
            void*    taskInputPtr    = increasePtrBySize(getArgPtr(arg), argInfo->taskIdx * getEntrySize(arg));
            uint64_t deviceInputAddr = argInfo->taskBufAddress + argInfo->inputBufOffset;
            memcpy(hostArgv, reinterpret_cast<void*>(&deviceInputAddr), sizeof(ARG*));
            memcpy(hostArgvPtr, taskInputPtr, getEntrySize(arg));
        }
        else
        {
            uint64_t deviceOutputAddr = argInfo->taskBufAddress + argInfo->outputBufOffset;
            memcpy(hostArgv, reinterpret_cast<void*>(&deviceOutputAddr), sizeof(ARG*));
        }
    }
    else
    {
        memset(hostArgv, 0, sizeof(uint64_t));
        memcpy(hostArgv, reinterpret_cast<void*>(&arg), sizeof(ARG));
    }
}

template <typename ARG, typename... ARGS>
void setupArg(ArgInfo_t* argInfo, ARG& arg, ARGS&... args)
{
    void* hostArgv = increasePtrBySize(argInfo->hostBuf, argInfo->argc * sizeof(uint64_t));
    if (IsMetisxPtr<ARG>::value)
    {
        size_t entrySize = getEntrySize(arg);
        if (isInputArg(arg))
        {
            void*    hostArgvPtr     = increasePtrBySize(argInfo->hostBuf, argInfo->inputBufOffset);
            void*    taskInputPtr    = increasePtrBySize(getArgPtr(arg), argInfo->taskIdx * entrySize);
            uint64_t deviceInputAddr = argInfo->taskBufAddress + argInfo->inputBufOffset;
            memcpy(hostArgv, reinterpret_cast<void*>(&deviceInputAddr), sizeof(ARG*));
            memcpy(hostArgvPtr, taskInputPtr, entrySize);
            argInfo->inputBufOffset += entrySize;
        }
        else
        {
            uint64_t deviceOutputAddr = argInfo->taskBufAddress + argInfo->outputBufOffset;
            memcpy(hostArgv, reinterpret_cast<void*>(&deviceOutputAddr), sizeof(ARG*));
            argInfo->outputBufOffset += entrySize;
        }
    }
    else
    {
        memset(hostArgv, 0, sizeof(uint64_t));
        memcpy(hostArgv, reinterpret_cast<void*>(&arg), sizeof(ARG));
    }
    argInfo->argc++;
    return setupArg(argInfo, args...);
}

template <typename ARG>
void getOutput(metisx::api::wrapper::Task* task, uint64_t taskIdx, uint64_t inputBufAddress, uint64_t inputOffset, void* outputBuf, ARG& arg)
{
    if (IsMetisxPtr<ARG>::value)
    {
        size_t entrySize    = getEntrySize(arg);
        void*  argBufferPtr = increasePtrBySize(getArgPtr(arg), taskIdx * entrySize);
        if (isInputArg(arg) && isOutputArg(arg))
        {
            task->readTaskBuffer(argBufferPtr, reinterpret_cast<void*>(inputBufAddress + inputOffset), 0, entrySize);
            inputOffset += entrySize;
        }
        else if (!isInputArg(arg) && isOutputArg(arg))
        {
            memcpy(argBufferPtr, outputBuf, entrySize);
            outputBuf = increasePtrBySize(outputBuf, entrySize);
        }
    }
}

template <typename ARG, typename... ARGS>
void getOutput(metisx::api::wrapper::Task* task, uint64_t taskIdx, uint64_t inputBufAddress, uint64_t inputOffset, void* outputBuf, ARG& arg, ARGS&... args)
{
    if (IsMetisxPtr<ARG>::value)
    {
        size_t entrySize    = getEntrySize(arg);
        void*  argBufferPtr = increasePtrBySize(getArgPtr(arg), taskIdx * entrySize);
        if (isInputArg(arg) && isOutputArg(arg))
        {
            task->readTaskBuffer(argBufferPtr, reinterpret_cast<void*>(inputBufAddress + inputOffset), 0, entrySize);
            inputOffset += entrySize;
        }
        else if (!isInputArg(arg) && isOutputArg(arg))
        {
            memcpy(argBufferPtr, outputBuf, entrySize);
            outputBuf = increasePtrBySize(outputBuf, entrySize);
        }
    }
    return getOutput(task, taskIdx, inputBufAddress, inputOffset, outputBuf, args...);
}
} // namespace detail
} // namespace Impl
} // namespace api
} // namespace metisx
