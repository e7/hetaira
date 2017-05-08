#ifndef __CRACK97_SMART_POINTER_H__
#define __CRACK97_SMART_POINTER_H__

#include <pthread.h>

#include "common.h"


template <typename TYPE> class e7::common::smart_pointer
{
public:
    static smart_pointer<TYPE> null_pointer;

public:
    smart_pointer<TYPE>(object *obj = NULL)
        : __obj__(NULL), __count_mutex__(NULL)
    {
        if (NULL == obj) {
            return;
        }

        e7::common::inheritance_ship<object, TYPE> inh;
        if (! inh) {
            assert(0);
        }
        __obj__ = reinterpret_cast<TYPE *>(obj);

        this->__count_mutex__ = __obj__->__count_mutex__;

        assert(0 == pthread_mutex_lock(this->__count_mutex__));
        __obj__->__ref_increase__();
        assert(0 == pthread_mutex_unlock(this->__count_mutex__));

        return;
    }

    // stl某些容器要求拷贝构造函数不能使用explicit
    smart_pointer<TYPE>(smart_pointer<TYPE> const &other)
        : __obj__(NULL), __count_mutex__(NULL)
    {
        *this = other;
    }

    smart_pointer<TYPE> const &operator =(smart_pointer<TYPE> const &other)
    {
        __release__();

        if (other.not_null()) {
            this->__obj__ = other.__obj__;
            this->__count_mutex__ = other.__count_mutex__;

            assert(0 == pthread_mutex_lock(this->__count_mutex__));
            this->__obj__->__ref_increase__();
            assert(0 == pthread_mutex_unlock(this->__count_mutex__));
        }

        return *this;
    }

    virtual ~smart_pointer<TYPE>(void)
    {
        __release__();
    }

public:
    intptr_t operator ==(void *p) const
    {
        return p == __obj__;
    }
    intptr_t operator !=(void *p) const
    {
        return p != __obj__;
    }
    intptr_t operator ==(smart_pointer const &other) const
    {
        return other.__obj__ == this->__obj__;
    }
    intptr_t operator !=(smart_pointer const &other) const
    {
        return other.__obj__ != this->__obj__;
    }
    intptr_t is_null(void) const
    {
        return NULL == __obj__;
    }
    intptr_t not_null(void) const
    {
        return NULL != __obj__;
    }
    TYPE *operator ->(void)
    {
        return __obj__;
    }
    TYPE &operator *(void)
    {
        return *__obj__;
    }
    void release(void)
    {
        __release__();

        return;
    }

    void copy(smart_pointer<TYPE> const &other)
    {
        __release__();
        if (other.not_null()) {
            __obj__ = new TYPE(*static_cast<smart_pointer>(other));
            __count_mutex__ = __obj__->__count_mutex__;

            assert(0 == pthread_mutex_lock(__count_mutex__));
            __obj__->__ref_increase__();
            assert(0 == pthread_mutex_unlock(__count_mutex__));
        }

        return;
    }

private:
    void __release__(void)
    {
        intptr_t ref_count = 0;

        if (NULL == this->__obj__) {
            return;
        }

        assert(0 == pthread_mutex_lock(this->__count_mutex__));
        __obj__->__ref_decrease__();
        ref_count = __obj__->__get_ref_count__();
        if (0 == ref_count) {
            delete __obj__;
        }
        __obj__ = NULL;
        assert(0 == pthread_mutex_unlock(this->__count_mutex__));

        if (0 == ref_count) {
            pthread_mutex_destroy(this->__count_mutex__);
            free(this->__count_mutex__);
        }
        this->__count_mutex__ = NULL;

        return;
    }

private:
    TYPE *__obj__;
    pthread_mutex_t *__count_mutex__;
};

template <typename TYPE> typename e7::common::smart_pointer<TYPE>
e7::common::smart_pointer<TYPE>::null_pointer;
#endif // __CRACK97_SMART_POINTER_H__
