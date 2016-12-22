#ifndef __SQLAGENT_CONC_QUEUE_H__
#define __SQLAGENT_CONC_QUEUE_H__


#include "common.h"

#include <list>


namespace e7 {
namespace common {
namespace conc_queue_ {
    class conc_queue;
}
typedef conc_queue_::conc_queue conc_queue;
} // end of namespace common
} // end of namespace e7


class e7::common::conc_queue_::conc_queue : public e7::common::object
{
    friend class e7::common::smart_pointer<conc_queue>;

public:
    explicit conc_queue(ssize_t max_sz);
    virtual ~conc_queue(void);

public:
    int is_full(void)
    {
        return (this->queue.size() < this->capacity) ? false : true;
    }
    ssize_t size(void)
    {
        return this->queue.size();
    }
    void push(void *p);
    void *pop(void);

private:
    ssize_t capacity;
    pthread_mutex_t mutex;
    std::list<void *> queue;
};
#endif // __SQLAGENT_CONC_QUEUE_H__
