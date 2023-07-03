#pragma once
#include <stdint.h>
#include <iostream>
#include <type_traits>
#include <assert.h>
namespace metisx
{
namespace api
{

enum class ArgType : std::size_t
{
    None   = 0x0,
    Input  = 0x1,
    Output = 0x2,
    Both   = 0x3,
};

namespace detail
{
typedef struct
{
        size_t  entrySize;
        size_t  entryCount;
        ArgType type;
} MetisxPtrHeader_t;

} // namespace detail

template <typename T>
class Metisx_ptr
{
    public:
        explicit Metisx_ptr(T* ptr=nullptr, bool debug=false)
        {
            dataPtr_  = ptr;
            refCount_ = new int(1);
            debug_    = debug;
        }

        Metisx_ptr(const Metisx_ptr<T>& other)
        {
            dataPtr_  = other.dataPtr_;
            refCount_ = other.refCount_;
            debug_    = other.debug_;

            if (refCount_ != nullptr)
            {
                (*refCount_)++;
                if (debug_)
                    std::cout << "MxPointer copy occurred..., refCount = " << *refCount_ << std::endl;
            }
        }

        Metisx_ptr& operator=(const Metisx_ptr& other) noexcept
        {
            if (this != &other)
            {
                release();
                dataPtr_  = other.dataPtr_;
                refCount_ = other.refCount_;
                if (refCount_)
                {
                    (*refCount_)++;

                    if (debug_)
                        std::cout << "MxPointer operator = , refCount = " << *refCount_ << std::endl;
                }
            }
        }

        void reset() const noexcept
        {
            *refCount_       = 0;
            void* releasePtr = getPtrHeader();
            free(releasePtr);
            dataPtr_ = nullptr;
            if (debug_)
                std::cout << "MxPointer reset..." << std::endl;
        }

        T* get() const noexcept
        {
            return dataPtr_;
        }

        T& operator*() const noexcept
        {
            return *dataPtr_;
        }

        T* operator->() const noexcept
        {
            return dataPtr_;
        }

        T& operator[](size_t __i) const
        {
            uint64_t addr = reinterpret_cast<uint64_t>(reinterpret_cast<void*>(get()));
            addr += __i * sizeof(T);
            return *reinterpret_cast<T*>(addr);
        }

        friend std::ostream& operator<<(std::ostream&  os,
                                        Metisx_ptr<T>& sp)
        {
            os << sp.get();
            return os;
        }

        ~Metisx_ptr()
        {
            release();
        }

    private:
        void* getPtrHeader()
        {
            return reinterpret_cast<void*>(reinterpret_cast<uint64_t>(dataPtr_) - sizeof(detail::MetisxPtrHeader_t));
        }

        void release()
        {
            if (refCount_)
            {
                (*refCount_)--;

                if (debug_)
                    std::cout << "MxPointer ref destroyed... refCont = " << *refCount_ << std::endl;

                if (*refCount_ == 0)
                {
                    void* releasePtr = getPtrHeader();
                    free(releasePtr);
                    delete refCount_;

                    dataPtr_  = nullptr;
                    refCount_ = nullptr;

                    if (debug_)
                        std::cout << "MxPointer released..." << std::endl;
                }
            }
        }

        int* refCount_;
        T*   dataPtr_;
        bool debug_;
};

template <typename T>
Metisx_ptr<T> make_metisx(std::size_t count, ArgType argType, bool debug = false)
{
    switch (argType)
    {
    case ArgType::Input:
        break;
    case ArgType::Output:
        break;
    case ArgType::Both:
        break;
    default:
        {
            assert(0);
        }
    }

    void* allocated                                                     = malloc(count * sizeof(T) + sizeof(detail::MetisxPtrHeader_t));
    reinterpret_cast<detail::MetisxPtrHeader_t*>(allocated)->entryCount = count;
    reinterpret_cast<detail::MetisxPtrHeader_t*>(allocated)->entrySize  = sizeof(T);
    reinterpret_cast<detail::MetisxPtrHeader_t*>(allocated)->type       = argType;
    allocated                                                           = reinterpret_cast<void*>(reinterpret_cast<uint64_t>(allocated) + sizeof(detail::MetisxPtrHeader_t));
    return Metisx_ptr<T>(static_cast<T*>(allocated), debug);
}

} // namespace api
} // namespace metisx
