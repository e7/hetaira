#ifndef __SQLAGENT_ACCEPTOR_THREAD_H__
#define __SQLAGENT_ACCEPTOR_THREAD_H__


#include "reciever_thread.h"


namespace sqlagent {
namespace acceptor_grp_arg_ {
    class acceptor_grp_arg;
}
typedef acceptor_grp_arg_::acceptor_grp_arg acceptor_grp_arg;

namespace acceptor_arg_ {
    class acceptor_arg;
}
typedef acceptor_arg_::acceptor_arg acceptor_arg;
}


namespace sqlagent {
extern volatile uintptr_t sig_quit; // 退出信号标识


class acceptor_grp_arg_::acceptor_grp_arg : public e7::common::object
{
    friend class e7::common::smart_pointer<acceptor_grp_arg>;

public:
    explicit acceptor_grp_arg(int listen_port, int thread_num, uint32_t proto)
    {
        this->port = listen_port;
        this->nthread = thread_num;
        this->business_type = proto;
    }

    int port;
    int nthread;
    uint32_t business_type;
};


class acceptor_arg_::acceptor_arg : public e7::common::object
{
    friend class e7::common::smart_pointer<acceptor_arg>;

public:
    explicit acceptor_arg(int listen_fd, uint32_t proto)
    {
        this->lsn_fd = listen_fd;
        this->business_type = proto;
    }
    int lsn_fd;
    uint32_t business_type;
};
}


extern void *acceptor_group_thread(void *arg);
#endif // __SQLAGENT_ACCEPTOR_THREAD_H__
