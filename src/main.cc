#include <sys/time.h>
#include <signal.h>
#include <linux/input.h>

#include "conf.h"
#include "pool.h"
#include "protocol.h"
#include "conc_queue.h"
#include "timer.h"
#include "singleton.h"
#include "socket.h"
#include "raii.h"
#include "acceptor_thread.h"
#include "reciever_thread.h"


#define TIMER_RESOLUTION        1000


using e7::common::smart_pointer;
using e7::common::fd_release;
using e7::common::single_raii;
using e7::common::obj_pool;
using e7::common::conc_queue;
using e7::common::proto_sjsonb;


namespace hetaira {
pthread_t tid_sig; // 信号线程id
pthread_t tid_oaccpt_grp; // outer-acceptor线程组
pthread_t tid_iaccpt_grp; // inner-acceptor线程组
pthread_t tids_recv[N_RECIEVERS]; // reciever线程id
int reciever_chans[N_RECIEVERS][2]; // reciever线程组通道

volatile uintptr_t sig_quit; // 退出信号标识
volatile uint64_t sig_now; // 当前时间
conc_queue *ct_idle_channel; // 空闲通道队列
conc_queue *ct_broken_conn; // 后端线程报告的损坏的连接队列
e7::common::singleton_mng ct_singleton_mng;


static void sigalrm_handler(int signum)
{
    static_cast<void>(::__sync_add_and_fetch (&sig_now, 1));

    return;
}

static void sigint_handler(int signum)
{
    while (! ::__sync_bool_compare_and_swap(&sig_quit, 0, 1)) {
    }
    return;
}

static void sigquit_handler(int signum)
{
    while (! ::__sync_bool_compare_and_swap(&sig_quit, 0, 1)) {
    }
}

static void sigpipe_handler(int signum)
{
    return;
}


static int init_signal(void)
{
    struct sigaction sa;

    sa.sa_flags = 0;

    sa.sa_handler = sigalrm_handler;
    if (-1 == ::sigaction(SIGALRM, &sa, NULL)) {
        return -1;
    }

    sa.sa_handler = sigint_handler;
    if (-1 == ::sigaction(SIGINT, &sa, NULL)) {
        return -1;
    }

    sa.sa_handler = sigint_handler;
    if (-1 == ::sigaction(SIGQUIT, &sa, NULL)) {
        return -1;
    }

    sa.sa_handler = sigpipe_handler;
    if (-1 == ::sigaction(SIGPIPE, &sa, NULL)) {
        return -1;
    }

    return 0;
}


void *signal_thread(void *)
{
    sigset_t denied;

    if (-1 == init_signal()) {
        return NULL;
    }

    // 设置允许的信号
    ::sigfillset(&denied);
    ::sigdelset(&denied, SIGABRT);
    ::sigdelset(&denied, SIGINT);
    ::sigdelset(&denied, SIGQUIT);
    ::sigdelset(&denied, SIGALRM);
    ::sigdelset(&denied, SIGSEGV);
    ::sigdelset(&denied, SIGPIPE);

    // 等待信号
    while (true) {
        ::sigsuspend(&denied);

        if (sig_quit) {
            break;
        }
    }

    return NULL;
}


int main(int argc, char *argv[], char *env[])
{
    int rslt, tmperr;
    struct timeval tv;
    struct itimerval itm;

    // 屏蔽所有信号
    sigset_t full, old;
    sigfillset(&full);
    sigemptyset(&old);
    sigprocmask(SIG_SETMASK, &full, &old);

    // 初始化定时器
    itm.it_interval = (struct timeval){0, 100 * TIMER_RESOLUTION};
    itm.it_value = (struct timeval){0, 100 * TIMER_RESOLUTION};

    rslt = ::gettimeofday(&tv, NULL);
    if (-1 == rslt) {
        return EXIT_FAILURE;
    }
    sig_now = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    rslt = ::setitimer(ITIMER_REAL, &itm, NULL);
    if (-1 == rslt) {
        return EXIT_FAILURE;
    }

    // 初始化随机种子
    ::srand(sig_now | 0x938F427A);

    // 创建信号线程
    tmperr = ::pthread_create(&tid_sig, NULL, &signal_thread, NULL);
    if (tmperr) {
        return EXIT_FAILURE;
    }

    // 创建inner监听线程组
    tmperr = ::pthread_create(
        &tid_iaccpt_grp, NULL, &acceptor_group_thread,
        new acceptor_grp_arg(8054, N_ACCEPTORS, hetaira::BUSI_INNER)
    );
    if (tmperr) {
        return EXIT_FAILURE;
    }

    // 创建reciever线程
    for (int i = 0; i < N_RECIEVERS; ++i) {
        if (-1 == ::socketpair(AF_UNIX, SOCK_STREAM, 0, reciever_chans[i])) {
            return EXIT_FAILURE;
        }
        (void)E7_SET_NONBLOCK(reciever_chans[i][0]);
        (void)E7_SET_NONBLOCK(reciever_chans[i][1]);
    }
    for (int i = 0; i < N_RECIEVERS; ++i) {
        // 奇数给reciever线程
        tmperr = ::pthread_create(
            &tids_recv[i], NULL, &reciever_thread,
            new reciever_arg(reciever_chans[i][1], new dft_action)
        );
        if (tmperr) {
            return EXIT_FAILURE;
        }
    }

    // 等待子线程结束，回收资源
    for (int i = 0; i < N_RECIEVERS; ++i) {
        pthread_join(tids_recv[i], NULL);
    }
    pthread_join(tid_iaccpt_grp, NULL);
    pthread_join(tid_oaccpt_grp, NULL);
    pthread_join(tid_sig, NULL);

    sigprocmask(SIG_SETMASK, &old, NULL);

    return EXIT_SUCCESS;

}
}


int main(int argc, char *argv[], char *env[])
{
    int rslt = 0;

    // 全局初始化
    hetaira::sig_quit = false;
    hetaira::sig_now = 0;

    hetaira::naked_conn_map = new hetaira::naked_conn_map_t[N_RECIEVERS];
    hetaira::ct_singleton_mng.append(
        "hetaira.naked_conn_map", hetaira::naked_conn_map
    );

    // run
    rslt = hetaira::main(argc, argv, env);

    return rslt;
}
