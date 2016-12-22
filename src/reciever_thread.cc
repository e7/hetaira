#include <sys/epoll.h>
#include <sys/socket.h>
#include <signal.h>
#include "reciever_thread.h"

#include "conf.h"
#include "raii.h"
#include "business/inner_cmnc.h"


namespace sqlagent {
extern volatile uintptr_t sig_quit; // 退出信号标识
}


using e7::common::fd_release;
using e7::common::dela_release;
using e7::common::single_raii;
using e7::common::array_raii;

using sqlagent::reciever_arg;
using sqlagent::connection;
using sqlagent::naked_conn_map;


void *reciever_thread(void *arg)
{
    int reactor;
    smart_pointer<reciever_arg> args(reinterpret_cast<reciever_arg *>(arg));

    reactor = ::epoll_create1(0);
    if (-1 == reactor) {
        return NULL;
    }
    single_raii<int, fd_release> auto_close_reactor(reactor);

    // 事件循环
    int nevents;
    int ee_cache_sz = MAX_CONNECTIONS * (N_ACCEPTORS * 2);
    epoll_event *ee_cache = new epoll_event[ee_cache_sz];
    array_raii< epoll_event *, dela_release<epoll_event> > auto_free_ee_cache(
        ee_cache, ee_cache_sz
    );

    epoll_event ee_chan = {
        EPOLLIN, {fd:args->channel}
    };
    ASSERT(0 == epoll_ctl(reactor, EPOLL_CTL_ADD, args->channel, &ee_chan));

    while (! sqlagent::sig_quit) {
        nevents = ::epoll_wait(reactor, ee_cache, ee_cache_sz, 20);
        if (-1 == nevents) {
            continue;
        }

        if (0 == nevents) {
            // timeout
            continue;
        }

        for (int i = 0; i < nevents; ++i) {
            if (ee_cache[i].data.fd == args->channel) {
                epoll_event ee_tmp;
                connection *new_conn;

                ASSERT(
                    ::recv(args->channel, &new_conn, sizeof(new_conn), 0) > 0
                );

                // 初始化连接，连接分为裸连接和session连接
                ASSERT(0 == E7_SET_NONBLOCK(new_conn->sock));

                if (-1 == new_conn->init(reactor)) {
                    smart_pointer<sqlagent::connection> auto_free(new_conn);
                    continue;
                }

                // 设置定时器回调

                // 连接回调
                args->act->on_connected(new_conn);

                continue;
            }

            connection *conn = reinterpret_cast<connection *>(
                ee_cache[i].data.ptr
            );
            if (ee_cache[i].events & (EPOLLHUP | EPOLLERR)) {
                smart_pointer<sqlagent::connection> auto_free(conn);
                args->act->on_closing(conn);
                continue;
            }
            if (ee_cache[i].events & EPOLLRDHUP) {
                // 对方断开连接
                smart_pointer<sqlagent::connection> auto_free(conn);
                args->act->on_closing(conn);
                continue;
            }

            args->act->on_recv(conn);
        }
    }

    return NULL;
}


namespace sqlagent {
void dft_action::on_connected(connection *conn)
{
    fprintf(stderr, "new connection\n");
}


void dft_action::on_closing(connection *conn)
{
    fprintf(stderr, "connection closed\n");
}


void dft_action::on_recv(connection *conn)
{
    ssize_t recv_sz;

    recv_sz = ::recv(conn->sock, &conn->rbuf.b[conn->rbuf.last],
                     conn->rbuf.space_left(), 0);
    if (-1 == recv_sz) {
        fprintf(stderr, "[ERROR] recv failed: %d\n", errno);
    }

    conn->rbuf.last += recv_sz; // 调整空闲空间位置

    switch (conn->business_type) {
    case BUSI_INNER:
        inner_service(conn);
        break;
    default:
        ASSERT(0);
        break;
    }

    return;
}
}
