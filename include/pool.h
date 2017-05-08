#ifndef __HETAIRA_POOL_H__
#define __HETAIRA_POOL_H__


#include "smart_pointer.h"


namespace e7 {
namespace common {
template <typename T> class obj_pool; // 非线程安全对象池

template <typename T>
class obj_pool : public deny_copyable
{
public:
    explicit obj_pool(ssize_t sz)
    {
        ASSERT(sz > 0);
        this->objs = new T[sz];

        this->idx_current = 0;
        this->last = sz - 1;
    }

    virtual ~obj_pool<T>(void)
    {
        delete []this->objs;
    }


public:
    T *borrow(void)
    {
        return this->objs[this->idx_current++];
    }

    T *data(void)
    {
        return this->objs;
    }

    ssize_t size(void)
    {
        ASSERT((this->last + 1) > 0);
        return this->last + 1;
    }

    void release(T *obj)
    {
        ASSERT_NOT(obj < &this->objs[0]);
        ASSERT_NOT(obj > &this->objs[last]);
        --idx_current;
    }


private:
    ssize_t idx_current;
    ssize_t last;
    T *objs;
};
} // end of namespace common
} // end of namespace e7
#endif // __HETAIRA_POOL_H__
