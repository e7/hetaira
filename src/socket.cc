#include "socket.h"
#include "singleton.h"


namespace sqlagent {
naked_conn_map_t *naked_conn_map;


int connection::init(int evlp_fd)
{
    epoll_event ee;

    this->reactor = evlp_fd;
    ee.events = EPOLLIN | EPOLLRDHUP;
    ee.data.ptr = this;
    if (-1 == ::epoll_ctl(reactor, EPOLL_CTL_ADD, this->sock, &ee)) {
        return -1;
    }

    return 0;
}


connection::~connection(void)
{
    if (-1 != this->reactor) {
        epoll_event ee;

        static_cast<void>(
            ::epoll_ctl(this->reactor, EPOLL_CTL_DEL, this->sock, &ee)
        );
    }
    static_cast<void>(::close(this->sock));

    return;
}
}
