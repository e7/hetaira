#ifndef __DB_THREAD_H__
#define __DB_THREAD_H__


#include "raii.h"
#include "smart_pointer.h"
#include "cJSON.h"


namespace sqlagent {
uint32_t const SA_CMD_REQ = 0x1234;
uint32_t const SA_CMD_QUIT = 0x1235;

namespace backend_arg_ {
    class backend_arg;
}
typedef backend_arg_::backend_arg backend_arg;

namespace req_body_ {
    class req_body;
}
typedef req_body_::req_body req_body;


class backend_arg_::backend_arg : public e7::common::object
{
    friend class e7::common::smart_pointer<backend_arg>;

public:
    explicit backend_arg(int fd)
    {
        this->channelfd = fd;
    }
    virtual ~backend_arg(void)
    {}

    int channelfd;
};


class req_body_::req_body : public e7::common::object
{
    friend class e7::common::smart_pointer<req_body>;

public:
    explicit req_body(uint32_t command, int fd, cJSON *p)
        : cmd(command), connfd(fd), req(p)
    {}
    virtual ~req_body(void) {::cJSON_Delete(this->req);}
    uint32_t cmd;
    int connfd;
    cJSON *req;
};


extern void *backend_proc(void *arg);
}
#endif // __DB_THREAD_H__
