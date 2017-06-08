// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



#ifndef LOCK_H_
#define LOCK_H_

#include <unistd.h>
#include <mach/mach_time.h>

//#include "comm/assert/__assert.h"
//#include "comm/thread/mutex.h"
//#include "comm/thread/spinlock.h"
//#include "comm/time_utils.h"

#include "mutex.h"
#include "spinlock.h"

template <typename MutexType>
class BaseScopedLock {
  public:
    explicit BaseScopedLock(MutexType& mutex, bool initiallyLocked = true)
        : mutex_(mutex) , islocked_(false) {
        if (!initiallyLocked) return;

        lock();
    }

    explicit BaseScopedLock(MutexType& mutex, long _millisecond)
        : mutex_(mutex) , islocked_(false) {
        timedlock(_millisecond);
    }

    ~BaseScopedLock() {
        if (islocked_) unlock();
    }

    bool islocked() const {
        return islocked_;
    }

    void lock() {
//        ASSERT(!islocked_);

        if (!islocked_ && mutex_.lock()) {
            islocked_ = true;
        }

//        ASSERT(islocked_);
    }

    void unlock() {
//        ASSERT(islocked_);

        if (islocked_) {
            mutex_.unlock();
            islocked_ = false;
        }
    }

    bool trylock() {
        if (islocked_) return false;

        islocked_ = mutex_.trylock();
        return islocked_;
    }
    
    uint64_t gettickcount() {
        static mach_timebase_info_data_t timebase_info = {0};
        
        // Convert to nanoseconds - if this is the first time we've run, get the timebase.
        if (timebase_info.denom == 0 )
        {
            (void) mach_timebase_info(&timebase_info);
        }
        
        // Convert the mach time to milliseconds
        uint64_t mach_time = mach_absolute_time();
        uint64_t millis = (mach_time * timebase_info.numer) / (timebase_info.denom * 1000000);
        return millis;
    }

#ifdef __linux__
    bool timedlock(long _millisecond) {
        ASSERT(!islocked_);

        if (islocked_) return true;

        islocked_ = mutex_.timedlock(_millisecond);
        return islocked_;
    }
#else
    bool timedlock(long _millisecond) {
//        ASSERT(!islocked_);

        if (islocked_) return true;

        unsigned long start = gettickcount();
        unsigned long cur = start;

        while (cur <= start + _millisecond) {
            if (trylock()) break;

            usleep(50 * 1000);
            cur = gettickcount();
        }

        return islocked_;
    }
#endif

    MutexType& internal() {
        return mutex_;
    }

  private:
    MutexType& mutex_;
    bool islocked_;
};

typedef BaseScopedLock<Mutex> ScopedLock;
typedef BaseScopedLock<SpinLock> ScopedSpinLock;

#endif /* LOCK_H_ */
