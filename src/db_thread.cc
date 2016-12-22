#include <sys/types.h>
#include <sys/socket.h>

#include "db_thread.h"
#include "protocol.h"


using e7::common::single_raii;
using e7::common::array_raii;
using e7::common::smart_pointer;
using e7::common::dels_release;
using e7::common::dela_release;
using e7::common::buffer;
using e7::common::proto_sjsonb;


void *sqlagent::backend_proc(void *arg)
{
    int maxfd;
    req_body *req;
    ssize_t ssz;
    smart_pointer<backend_arg> sp_arg(reinterpret_cast<backend_arg *>(arg));

    maxfd = sp_arg->channelfd;
    while (true) {
        int n_ready;
        fd_set rset;

        FD_ZERO(&rset);
        FD_SET(sp_arg->channelfd, &rset);
        n_ready = ::select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (n_ready <= 0) {
            continue;
        }

        for (int i = 0; i < n_ready; ++i) {
            if (FD_ISSET(sp_arg->channelfd, &rset)) {
                // 内部通信忽略返回值
                static_cast<void>(
                    ::recv(sp_arg->channelfd, &req, sizeof(void *), 0)
                );
            }
        }

        ASSERT(req);
        single_raii< req_body *, dels_release<req_body> > free_req(req);
        if (req->cmd == SA_CMD_QUIT) {
            fprintf(stderr, "backend thread exit\n");
            break;
        }

        smart_pointer<buffer> pkg(proto_sjsonb::encode(req->req));

        ssz = ::send(req->connfd, pkg->b, pkg->length, 0);
    }

    return NULL;
}


