/**
 *  @file   mutex.h
 *  @author ichenq@gmail.com
 *  @date   May 25, 2011
 *  @brief  mutex for thread synchronization
 */

#pragma once

#include "utility.h"
#include <intrin.h>



//
// a busy waiting spin lock
//

#define CACHE_SIZE  64

class spinlock
{
    enum {_UNLOCKED = 0, _LOCKED = 1};

public:
    spinlock()
    {
        memset(this, 0, sizeof(*this));
    }

    bool try_lock()
    {
        bool r = (::InterlockedExchange(&lock_, _LOCKED) == _UNLOCKED);
        _ReadWriteBarrier();
        return r;
    }

    void lock()
    {
        for (unsigned i = 0; !try_lock(); ++i)
        {
            yield(i);
        }
    }

    void unlock()
    {
        _ReadWriteBarrier();
        *const_cast<long volatile*>(&lock_) = _UNLOCKED;
    }

private:
    spinlock(const spinlock&);
    spinlock& operator = (const spinlock&);

    // @see http://www.boost.org/doc/libs/1_47_0/boost/smart_ptr/detail/yield_k.hpp
    void yield(unsigned k)
    {
        if (k < 4) {}
        // spin loop hint, @see http://siyobik.info/main/reference/instruction/PAUSE
        else if (k < 16) { _mm_pause();} 
        else if (k < 32) {::Sleep(0);}
        else {::Sleep(1);}
    }

    char    __padding1[CACHE_SIZE/2];
    long    lock_;
    char    __padding2[CACHE_SIZE/2-sizeof(long)];
};



//
// mutex using win32 critical section object
//
class mutex
{
public:
    mutex() 
    {
        ::InitializeCriticalSection(&cs_);
    }
    ~mutex()
    {
        ::DeleteCriticalSection(&cs_);
    }

    bool try_lock() 
    {
        return (::TryEnterCriticalSection(&cs_) == TRUE);
    }

    void lock() 
    {
        ::EnterCriticalSection(&cs_);
    }

    void unlock() 
    {
        ::LeaveCriticalSection(&cs_);
    }

private:
    mutex(const mutex&);
    mutex& operator = (const mutex&);

    CRITICAL_SECTION    cs_;
};



template <typename Lockable>
class scoped_lock
{
public:
    explicit scoped_lock(Lockable& lock)
        : lock_(lock)
    {
        lock_.lock();
    }

    ~scoped_lock() 
    {
        lock_.unlock();
    }

private:
    scoped_lock(const scoped_lock&);
    scoped_lock& operator = (const scoped_lock&);

    Lockable&  lock_;
};


