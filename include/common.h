#ifndef __SQLAGENT_COMMON_H__
#define __SQLAGENT_COMMON_H__


#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <map>
#include <vector>
#include <string>

#define ASSERT(e)       do {if (! (e)) {::abort();}} while (0)
#define ASSERT_NOT(e)   do {if (e) {::abort();}} while (0)
#define ARRAY_COUNT(a)  (sizeof(a) / sizeof(a[0]))


namespace e7 {
namespace common {
typedef std::map<std::string, std::string> str2str;

namespace deny_copyable_ {
    class deny_copyable;
}
typedef deny_copyable_::deny_copyable deny_copyable;

namespace object_ {
    class object;
}
typedef object_::object object;

template <typename base, typename derived> class inheritance_ship;
template <typename TYPE> class smart_pointer;
} // end of namespace common
} // end of namespace e7


template <typename base, typename derived> class e7::common::inheritance_ship
{
public:
    /* 检测继承关系，真表示继承关系成立 */
    bool operator ()(void) const
    {
        return (sizeof(check(static_cast<base const*>(0))) == \
            sizeof(check(static_cast<derived const*>(0))));
    }

    operator bool(void) const
    {
        return (*this)();
    }

private:
    /* type为无用模板形参，因为嵌套类模板只能偏特化 */
    template <int n, typename type=void> class __size_box__
    {
    private:
        __size_box__<n-1, type> __box1__;
        __size_box__<n-1, type> __box2__;
    };

    template <typename type> class __size_box__<0, type>
    {
    private:
        char __c__;
    };

private:
    __size_box__<0> check(base const*) const;
    __size_box__<1> check(...) const;

};


class e7::common::deny_copyable_::deny_copyable
{
protected:
    explicit deny_copyable(void) {}
    virtual ~deny_copyable(void) {}
    virtual int init(void)
    {
        return 0;
    }

private:
    deny_copyable(const deny_copyable &);
    deny_copyable const &operator =(const deny_copyable &);
}; // end of e7::common::deny_copyable_::deny_copyable


// 从object继承的对象被用于智能指针，而指针是无需
// 拷贝构造的，或者拷贝智能指针对象
class e7::common::object_::object : public e7::common::deny_copyable
{
     template <typename TYPE> friend class e7::common::smart_pointer;

public:
    explicit object(void)
        : __ref_count__(0), __count_mutex__(NULL)
    {
        __count_mutex__ = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(__count_mutex__, NULL);
    }

    explicit object(object const &other)
        : __ref_count__(0), __count_mutex__(NULL)
    {
        __count_mutex__ = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(__count_mutex__, NULL);
    }

    int init(void) {return 0;}

    virtual ~object(void)
    {}

private:
    void *operator new[](size_t size);
    void operator delete[](void *p);

private:
    // just for smart_pointer
    void __ref_increase__(void)
    {
        ++__ref_count__;
    }

    intptr_t __get_ref_count__(void) const
    {
        return __ref_count__;
    }

    void __ref_decrease__(void)
    {
        --__ref_count__;
    }

private:
    intptr_t __ref_count__;
    pthread_mutex_t *__count_mutex__; // 可用cas汇编指令或gcc内置原子操作替代
}; // end of e7::common::object_::object
#endif // __SQLAGENT_COMMON_H__
