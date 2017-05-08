#ifndef __SQLAGENT_RAII_H__
#define __SQLAGENT_RAII_H__

#include "common.h"


namespace e7 {
namespace common {
// 各种raii回调类
namespace fd_release_ {
    class fd_release;
}
typedef fd_release_::fd_release fd_release;

// 各种raii
template <typename T, typename R> class single_raii;
} // end of namespace common
}// end of namespace e7


class e7::common::fd_release_::fd_release
{
public:
    int operator ()(int fd)
    {
        return ::close(fd);
    }
};


// 即使不是继承object的类也可以被single_raii接管，然后被智能指针接管
template <typename T, typename R>
class e7::common::single_raii : public e7::common::object
{
    friend class e7::common::smart_pointer< single_raii<T, R> >;

public:
    single_raii(T p)
    {
        this->rsc = p;
    }

    virtual ~single_raii(void)
    {
        this->release(this->rsc);
    }

private:
    T rsc;
    R release;
}; // end of e7::common::single_raii
#endif // __SQLAGENT_RAII_H__
