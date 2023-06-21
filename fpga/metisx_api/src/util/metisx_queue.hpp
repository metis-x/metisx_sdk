#pragma once

#include <memory>
#include <mutex>
#include <assert.h>

namespace metisx
{

namespace api
{

namespace util
{
enum class QueueResponse
{
    PUSH_SUCCESS,
    PUSH_FAIL,
    POP_SUCCESS,
    POP_FAIL
};

template <class T>
class MxQueue
{

    public:
        explicit MxQueue() // 생성자
            : _capacity(0),
              _head(0),
              _tail(0)

        {
            _tailMutex = std::make_unique<std::mutex>();
            _headMutex = std::make_unique<std::mutex>();
        }

        virtual ~MxQueue()
        {
        }

        T& operator[](int idx)
        {
            if (idx >= size())
            {
                assert(0);
            }

            return _arr[(_head + idx) % _capacity];
        }

        void init(int size)
        {
            _capacity = size + 1;
            _arr      = std::make_unique<T[]>(_capacity);
        }

        QueueResponse pop(T& x)
        {
            if (_head == _tail)
            {
                return QueueResponse::POP_FAIL; // empty = can't pop an entry from queue
            }
            peek(x);
            _head = (_head + 1) % _capacity;
            return QueueResponse::POP_SUCCESS;
        }

        QueueResponse lock_pop(T& x)
        {
            std::lock_guard<std::mutex> _guard(*_headMutex);
            if (_head == _tail)
            {
                return QueueResponse::POP_FAIL; // empty = can't pop an entry from queue
            }
            x     = _arr[_head];
            _head = (_head + 1) % _capacity;
            return QueueResponse::POP_SUCCESS;
        }

        QueueResponse push(T x)
        {
            int nexTail = (_tail + 1) % _capacity;
            if (_head == nexTail)
            {
                return QueueResponse::PUSH_FAIL;
            }
            _arr[_tail] = x;
            _tail       = nexTail;
            return QueueResponse::PUSH_SUCCESS;
        }

        QueueResponse lock_push(T x)
        {
            std::lock_guard<std::mutex> _guard(*_tailMutex);
            int                         nexTail = (_tail + 1) % _capacity;
            if (_head == nexTail)
            {
                return QueueResponse::PUSH_FAIL;
            }
            _arr[_tail] = x;
            _tail       = nexTail;

            return QueueResponse::PUSH_SUCCESS;
        }

        void peek(T& x)
        {
            std::lock_guard<std::mutex> _guard(*_tailMutex);
            x = _arr[_head];
        }

        int size()
        {
            std::lock_guard<std::mutex> _guard(*_tailMutex);
            return (_tail + _capacity - _head) % _capacity;
        }

        int depth()
        {
            std::lock_guard<std::mutex> _guard(*_tailMutex);
            return _capacity;
        }

        bool isEmpty() const
        {
            std::lock_guard<std::mutex> _guard(*_tailMutex);
            return (_head == _tail);
        }

        bool isFull() const
        {
            std::lock_guard<std::mutex> _guard(*_tailMutex);
            int                         nexTail = (_tail + 1) % _capacity;
            return (_head == nexTail);
        }

    public:
        class Iterator
        {
            public:
                ~Iterator()
                {
                }

                // operator overloading
                Iterator& operator++()
                {
                    _next = (_next + 1) % _p->_capacity;
                    return *this;
                }

                const Iterator operator++(int)
                {
                    _next = (_next + 1) % _p->_capacity;
                    return *this;
                }

                bool operator!=(const Iterator& it) const
                {
                    T* v1 = &_p->_arr[_next];
                    T* v2 = &it._p->_arr[it._next];
                    if (v1 == v2)
                    {
                        return false;
                    }

                    return true;
                }

                T& operator*()
                {
                    return _p->_arr[_next];
                }

            private:
                friend class MxQueue;

                Iterator(MxQueue* p, uint64_t idx)
                    : _p(p),
                      _next(idx)
                {
                    _next = idx;
                }

            private:
                MxQueue* _p;
                int      _next;
        };

        Iterator begin()
        {
            return Iterator(this, _head);
        }

        Iterator end()
        {
            return Iterator(this, _tail);
        }

    private:
        std::unique_ptr<T[]>        _arr;
        int                         _capacity = 0;
        int                         _head     = 0;
        int                         _tail     = 0;
        std::unique_ptr<std::mutex> _tailMutex;
        std::unique_ptr<std::mutex> _headMutex;
};

} // namespace util
} // namespace api
} // namespace metisx
