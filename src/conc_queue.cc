#include "conc_queue.h"
#include "smart_pointer.h"


using e7::common::array_raii;
using e7::common::smart_pointer;


namespace e7 {
namespace common {
namespace guard_ {
    class guard;
}
typedef guard_::guard guard;


class guard_::guard : public e7::common::object
{
    friend class e7::common::smart_pointer<guard>;

public:
    explicit guard(pthread_mutex_t *lock)
    {
        ASSERT(lock);
        this->mtx = lock;
        ::pthread_mutex_lock(lock);
    }

    virtual ~guard(void)
    {
        ::pthread_mutex_unlock(this->mtx);
    }

private:
    pthread_mutex_t *mtx;
};
}
}


e7::common::conc_queue_::conc_queue::conc_queue(ssize_t max_sz)
{
    this->capacity = max_sz;
    ASSERT(0 == ::pthread_mutex_init(&this->mutex, NULL));
}


e7::common::conc_queue_::conc_queue::~conc_queue(void)
{
    ASSERT(0 == ::pthread_mutex_destroy(&this->mutex));
}


void e7::common::conc_queue_::conc_queue::push(void *p)
{
    e7::common::guard autolock(&this->mutex);

    ASSERT(this->queue.size() < this->capacity);
    this->queue.push_back(p);
}


void *e7::common::conc_queue_::conc_queue::pop(void)
{
    void *rslt;
    e7::common::guard autolock(&this->mutex);

    ASSERT(this->queue.size() > 0);
    rslt = this->queue.front();
    this->queue.pop_front();

    return rslt;
}
