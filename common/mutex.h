/**
 *  @file:   Mutex.h
 *  @brief:  
 *
 * 	@author: ichenq@gmail.com
 *  @date:   May 25, 2011
 */

#pragma once

#include "utility.h"
#include <intrin.h>


#pragma warning(push)
#pragma warning(disable: 4324)



//
// a busy waiting spin lock
//

#define CACHE_ALIGN __declspec(align(64))

class CACHE_ALIGN spinlock
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
        else if (k < 16) { _mm_pause();} // spin loop hint, @see http://siyobik.info/main/reference/instruction/PAUSE
        else if (k < 32) {::Sleep(0);}
        else {::Sleep(1);}
    }

    long    lock_;
};



//
// mutex using win32 critical section object
//
class mutex
{
public:
    // Constructor 
    // @brief InitializeCriticalSection() will raise an exception 
    //        if failed to allocate the critical section object,
    //        so InitializeCriticalSectionAndSpinCount() is more safe.
    // @param dwSpinCount The spin count for the critical section object
    explicit mutex(size_t spin_count = 0)
    {
        if (!::InitializeCriticalSectionAndSpinCount(&cs_, spin_count))
        {
            LOG_ERROR_MSG(_T("InitializeCriticalSectionAndSpinCount() failed"));
        }
    }

    ~mutex()
    {
        ::DeleteCriticalSection(&cs_);
    }

public:
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



template <typename LockT>
class scoped_lock
{
public:
    // Constructor
    // @param lock A reference to a lock object
    explicit scoped_lock(LockT& lock)
        : lock_(lock)
    {
        lock_.lock();
    }

    ~scoped_lock()
    {
        lock_.unlock();
    }

private:
    // noncopyable
    scoped_lock(const scoped_lock&);
    scoped_lock& operator = (const scoped_lock&);

    LockT&  lock_;
};

#pragma warning(pop)
