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
template <typename T> class dels_release;
template <typename T> class dela_release;
namespace cmem_release_ {
    class cmem_release;
}
typedef cmem_release_::cmem_release cmem_release;

// 各种raii
template <typename T, typename R> class single_raii;
template <typename T, typename R> class array_raii;
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


template <typename T>
class e7::common::dels_release
{
public:
    int operator ()(T *p)
    {
        delete p;
        return 0;
    }
};


template <typename T>
class e7::common::dela_release
{
public:
    int operator ()(T *p)
    {
        delete[] p;
        return 0;
    }
};


class e7::common::cmem_release_::cmem_release
{
public:
    int operator()(void *p)
    {
        free(p);
        return 0;
    }
};


// 即使不是继承object的类也可以被single_raii或array_raii
// 接管，然后被智能指针接管
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


// 即使不是继承object的类也可以被single_raii或array_raii
// 接管，然后就可以被智能指针接管
template <typename T, typename R>
class e7::common::array_raii : public e7::common::object_::object
{
    friend class e7::common::smart_pointer< array_raii<T, R> >;

public:
    array_raii(T obj_array, ssize_t n)
    {
        this->rsc = obj_array;
        this->length = n;
    }

    virtual ~array_raii(void)
    {
        this->release(this->rsc);
    }

    T at(ssize_t i)
    {
        ASSERT_NOT(i < 0);
        ASSERT_NOT(i >= this->length);

        return this->rsc[i];
    }

    T data(void)
    {
        return this->rsc;
    }

    ssize_t size(void)
    {
        return this->length;
    }

private:
    T rsc;
    ssize_t length;
    R release;
}; // end of template class e7::common::array_raii
#endif // __SQLAGENT_RAII_H__
