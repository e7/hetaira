#ifndef __SQLAGENT_BUFFER_H__
#define __SQLAGENT_BUFFER_H__


#include "smart_pointer.h"


namespace e7 {
namespace common {
namespace buffer_ {
    class buffer;
}
typedef buffer_::buffer buffer;


// [0, length) 缓冲总内存区
// [0, seek) 已处理的缓冲区
// [seek, last) 待处理的缓冲区
// [last, length) 可写入的缓冲区
class buffer_::buffer : public e7::common::object
{
    friend class e7::common::smart_pointer<buffer>;

public:
    uint8_t *b;
    ssize_t seek;
    ssize_t last;
    ssize_t length;

    explicit buffer(ssize_t n)
    {
        this->b = new uint8_t[n];
        this->seek = 0;
        this->last = 0;
        this->length = n;
    }
    virtual ~buffer(void)
    {
        delete[] b;
    }

    void reset(void)
    {
        this->seek = 0;
        this->last = 0;
    }
    ssize_t data_size(void)
    {
        return (this->last - this->seek);
    }

    ssize_t space_left(void)
    {
        return (this->length - this->last);
    }

    void spare(void)
    {
        if (this->space_left() * 4 < this->length) {
            this->_do_spare();
        }
    }

    void append(void *d, ssize_t len)
    {
        ::memcpy(&this->b[this->last], d, len);
        this->last += len;
    }


protected:
    void _do_spare(void)
    {
        ssize_t nmove = this->last - this->seek;
        ::memmove(&this->b[0], &this->b[this->seek], nmove);
        this->seek = 0;
        this->last -= nmove;
    }
}; // end of class buffer_::buffer
} // end of namespace common
} // end of namespace e7
#endif // __SQLAGENT_BUFFER_H__
