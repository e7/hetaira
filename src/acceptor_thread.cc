#include <signal.h>
#include "acceptor_thread.h"

#include "conf.h"
#include "raii.h"
#include "socket.h"


namespace hetaira {
extern int reciever_chans[N_RECIEVERS][2]; // reciever线程组通道
}

static void *acceptor_thread(void *arg);


using e7::common::fd_release;
using e7::common::single_raii;
using e7::common::smart_pointer;

using hetaira::connection;
using hetaira::acceptor_grp_arg;
using hetaira::acceptor_arg;
using hetaira::reciever_chans;


void *acceptor_group_thread(void *arg)
{
    smart_pointer<acceptor_grp_arg> auto_free_args(
        reinterpret_cast<acceptor_grp_arg *>(arg)
    );

    // 创建监听
    int lsn_fd;
    int reuseaddr = 1;
    socklen_t srv_len;
    struct sockaddr_in srv_addr;

    lsn_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == lsn_fd) {
        return NULL;
    }
    single_raii<int, fd_release> auto_close_lsn_fd(lsn_fd);

    if (-1 == E7_SET_NONBLOCK(lsn_fd)) {
        fprintf(stderr, "[ERROR] set non-block fd failed:%d\n", errno);
    }

    if (-1 == ::setsockopt(lsn_fd, SOL_SOCKET, SO_REUSEADDR,
                           &reuseaddr, sizeof(int))) {
        // log
    }

    ::memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = ::htons(auto_free_args->port); // 端口
    srv_len = sizeof(srv_addr);
    if (-1 == ::bind(
        lsn_fd, reinterpret_cast<sockaddr const *>(&srv_addr), srv_len)) {
        fprintf(stderr, "bind failed:%d\n", errno);
        return NULL;
    }

    if (-1 == ::listen(lsn_fd, SOMAXCONN)) {
        return NULL;
    }

    // 创建监听线程
    int tmperr;
    std::vector<pthread_t> tids(auto_free_args->nthread);

    for (int i = 0; i < auto_free_args->nthread; ++i) {
        tmperr = ::pthread_create(
            &tids[i], NULL, &acceptor_thread,
            new acceptor_arg(lsn_fd, auto_free_args->business_type)
        );
        if (tmperr) {
            fprintf(stderr, "start acceptor failed: %d\n", errno);
            --i; ::sleep(1); continue; // 尝试启动线程
        }
    }

    // 等待线程退出
    for (int i = 0; i < auto_free_args->nthread; ++i) {
        ::pthread_join(tids[i], NULL);
    }

    return NULL;
}


void *acceptor_thread(void *arg)
{
    int reactor;
    smart_pointer<acceptor_arg> auto_free_args(
        reinterpret_cast<acceptor_arg *>(arg)
    );

    reactor = ::epoll_create1(0);
    if (-1 == reactor) {
        return NULL;
    }
    single_raii<int, fd_release> auto_close_reactor(reactor);


    // 事件循环
    int nevents;
    int lsn_fd = auto_free_args->lsn_fd;
    struct epoll_event lsn_ee;
    struct epoll_event ee_cache[4];

    lsn_ee.events = EPOLLIN; // 水平触发
    lsn_ee.data.fd = lsn_fd;
    ::epoll_ctl(reactor, EPOLL_CTL_ADD, lsn_fd, &lsn_ee);
    while (! hetaira::sig_quit) {
        nevents = ::epoll_wait(reactor, ee_cache, ARRAY_COUNT(ee_cache), 20);
        if (-1 == nevents) {
            continue;
        }

        if (0 == nevents) {
            // timeout
            continue;
        }

        int connfd = ::accept(lsn_fd, NULL, NULL);
        if (-1 == connfd) {
            continue;
        }

        // dispatch
        connection *new_conn = (
            new connection(connfd, auto_free_args->business_type)
        );
        if (-1 == ::send(reciever_chans[connfd % N_RECIEVERS][0],
                         &new_conn, sizeof(new_conn), 0)) {
            smart_pointer<connection> auto_free_conn(new_conn);
            fprintf(stderr, "dispatch failed: %d\n", errno);
        }
    }

    return NULL;
}
