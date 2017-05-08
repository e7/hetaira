#ifndef __U2USH_TIMER_H__
#define __U2USH_TIMER_H__


#include <cstdint>

#include <queue>
#include <iostream>

#include "smart_pointer.h"


namespace u2ush {
namespace timer_heap_ {
class timer;
class i_on_timeout;
class cmp_timer;
class timer_heap;
}
typedef timer_heap_::timer timer;
typedef timer_heap_::i_on_timeout i_on_timeout;
typedef timer_heap_::cmp_timer cmp_timer;
typedef timer_heap_::timer_heap timer_heap;
}


namespace u2ush {
namespace timer_heap_ {
class cmp_timer
{
public:
    bool operator ()(e7::common::smart_pointer<timer> a,
                     e7::common::smart_pointer<timer> b);
};

class i_on_timeout : public e7::common::deny_copyable
{
public:
    virtual void do_trigger(void) = 0;
    virtual ~i_on_timeout(void) {}
};

class timer : public e7::common::object
{
public:
    explicit timer(uint64_t timeout, i_on_timeout *trigger)
        : timeout(timeout), trigger(trigger)
    {
    }

    virtual ~timer(void)
    {
        if (trigger) {
            delete trigger;
            trigger = NULL;
        }
    }

    uint64_t timeout;
    i_on_timeout *trigger;
};

class timer_heap : public e7::common::deny_copyable
{
public:
    std::priority_queue< e7::common::smart_pointer<timer>,
                         std::vector< e7::common::smart_pointer<timer> >,
                         cmp_timer> heap;
};
} // end of namespace u2ush::timer_heap_
} // end of namespace u2ush
#endif // __U2USH_TIMER_H__
