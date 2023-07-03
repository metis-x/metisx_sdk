#pragma once
#include "./wrapper/metisx_api_job_wrapper.hpp"
#include "./util/metisx_exception.hpp"
#include "metisx_api_map_impl.hpp"
#include "metisx_api_metisx_ptr.hpp"

namespace metisx
{
namespace api
{

namespace detail
{
template <typename String>
std::unique_ptr<metisx::api::wrapper::Job> allocateJob(const String& slaveMuBinary, size_t numThread = 0, size_t numSlaveMu = 0, uint64_t threadBitmap = 0)
{
    auto   offloadJob = std::make_unique<metisx::api::wrapper::Job>();
    try
    {
        offloadJob->init(metisx::api::wrapper::Job::InvalidJobId, numThread);
        offloadJob->setThreadBitmap(numSlaveMu, threadBitmap);
        offloadJob->loadProgram(slaveMuBinary);
    }
    catch (const metisx::api::util::MetisException& err)
    {
        std::cerr << err.what();
    }
    return std::move(offloadJob);
}
}

void* mxAllocHeap(size_t size);
void* mxLoadHeap(void* dstAddress, void* srcAddress, size_t size);
void  mxReleaseHeap(void* dstAddress);

template <typename String, typename... ARGS>
void mxMapDebug(const String& fileName, uint32_t numThread, uint32_t numSlaveMu, uint64_t threadBitmap, ARGS... args)
{
    std::unique_ptr<metisx::api::wrapper::Job> metisJob = detail::allocateJob(fileName, numThread, numSlaveMu, threadBitmap);
    impl::mxMapImpl(metisJob->getJobId(), args...);
    metisJob.reset();
}

template <typename String, typename... ARGS>
void mxMapOneMu(const String& fileName, ARGS... args)
{
    std::unique_ptr<metisx::api::wrapper::Job> metisJob = detail::allocateJob(fileName, 1, 1, 0xF);
    impl::mxMapImpl(metisJob->getJobId(), args...);
    metisJob.reset();
}

template <typename String, typename... ARGS>
void mxMap(const String& fileName, ARGS... args)
{
    std::unique_ptr<metisx::api::wrapper::Job> metisJob = detail::allocateJob(fileName);    
    impl::mxMapImpl(metisJob->getJobId(), args...);
    metisJob.reset();
}

} // namespace api
} // namespace metisx