#ifndef __SQLAGENT_SOCKET_H__
#define __SQLAGENT_SOCKET_H__


#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>

#include "smart_pointer.h"
#include "buffer.h"

#define E7_SET_NONBLOCK(fd)     \
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)


namespace sqlagent {
// 连接的业务类型
enum {
    BUSI_INNER = 1,
};


namespace connection_ {
    class connection;
}
typedef connection_::connection connection;


class connection_::connection : public e7::common::object
{
    friend class e7::common::smart_pointer<connection>;

public:
    int sock;
    int reactor;
    uint32_t business_type; // 该连接的业务类型
    e7::common::buffer rbuf;

    explicit connection(int fd, uint32_t proto) : rbuf(4096)
    {
        this->sock = fd;
        this->reactor = -1;
        this->business_type = proto;
    }

    int init(int evlp_fd);

    virtual ~connection(void);
};



typedef std::map< int, e7::common::smart_pointer<connection> > naked_conn_map_t;
extern naked_conn_map_t *naked_conn_map; // 裸连接集
} // end of namespace sqlagent
#endif // __SQLAGENT_SOCKET_H__
